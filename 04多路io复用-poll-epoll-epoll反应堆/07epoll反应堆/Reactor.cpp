#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>

#include "Reactor.h"
#include "wrap.h"

//#define __DEBUG

using namespace std;

//string EpollReactor::data;
stringstream EpollReactor::sdata;

EpollReactor::EpollReactor(
	const int& size, const int& maxevents, const int& timeout) :
	maxevents(maxevents), timeout(timeout)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif


	epollFd = epoll_create(size);

	int stat = fcntl(STDIN_FILENO, F_GETFL);
	fcntl(STDIN_FILENO, F_SETFL, stat | O_NONBLOCK);	
	stat = fcntl(STDOUT_FILENO, F_GETFL);
	fcntl(STDOUT_FILENO, F_SETFL, stat | O_NONBLOCK);

	addEvent(STDIN_FILENO, EPOLLIN | EPOLLET, stdinProc);
	addEvent(STDOUT_FILENO, EPOLLIN | EPOLLET, stdoutProc);

	events = (epoll_event*)malloc(sizeof(epoll_event) * maxevents);
}


EpollReactor::~EpollReactor()
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif

	close(epollFd);
	free(events);
}

EpollReactor::__Event * EpollReactor::checkEvent(const int & fd)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif

	std::map<int, __Event>::iterator it;
	if (eventMap.end() == (it = eventMap.find(fd)))return nullptr;
	return &it->second;
}

void EpollReactor::Run()
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif

	while (true)
	{
		//cout << "epoll_ wait" << endl;
		int n = epoll_wait(epollFd, events, maxevents, timeout);
		switch (n)
		{
		case -1:
			perror("epoll_wait err");
			return;
		case 0:
			printf("epoll_wait timeout");
			break;
		default:
			for (int i = 0; i < n; i++)
			{
				__Event* pe = (__Event*)events[i].data.ptr;
				pe->callback(pe->fd, pe->events, this);
			}
			break;
		}
	}
}

int EpollReactor::listen(const int & port)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif


	int lstnFd = Socket(AF_INET, SOCK_STREAM, 0);
	addEvent(lstnFd, EPOLLIN, lstnProc);
	int opt = 1;
	setsockopt(lstnFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	//inet_pton(AF_INET, "192.168.233.233", &sa.sin_addr.s_addr);
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);
	Bind(lstnFd, (sockaddr*)&sa, sizeof(sa));
	Listen(lstnFd, 128);


	return 0;
}


int EpollReactor::addEvent(
	const int & fd, const int & events,
	void(*const & callback)(int fd, int events, void *arg),
	void* const & data, const int& dataSize)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif

	if (eventMap.end() != eventMap.find(fd) || NULL == callback)
	{
		printf("param is failed...");
		return -1;
	}

	eventMap[fd] = { fd, events, callback, data, dataSize };

	epoll_event ee;
	ee.events = events;
	ee.data.ptr = &eventMap[fd];

	epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ee);


	return 0;
}

int EpollReactor::delEvent(const int & fd)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif

	std::map<int, __Event>::iterator it;
	if (eventMap.end() == (it = eventMap.find(fd))) return -1;

	close(fd);
	free(checkEvent(fd)->data);

	epoll_event ee;
	ee.data.ptr = NULL;
	ee.events = it->second.events;

	epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ee);

	eventMap.erase(it);

	return 0;
}

int EpollReactor::setEvent(
	const int & fd, const int & events, 
	void(*const & callback)(int fd, int events, void *arg),
	void* const & data, const int& dataSize)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif

	std::map<int, __Event>::iterator it;
	if (eventMap.end() == (it = eventMap.find(fd)) || NULL == callback)
		return -1;
	
	it->second.events = events;
	it->second.callback = callback;
	free(it->second.data);
	it->second.data = data;
	it->second.dataSize = dataSize;

	epoll_event ee;
	ee.events = events;
	ee.data.ptr = &eventMap[fd];

	epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ee);

	return 0;
}
void EpollReactor::pIP(void* data)
{
	//EpollReactor::data.append(to_string(((unsigned char*)data)[0]));
	//EpollReactor::data.append(".");
	//EpollReactor::data.append(to_string(((unsigned char*)data)[1]));
	//EpollReactor::data.append(".");
	//EpollReactor::data.append(to_string(((unsigned char*)data)[2]));
	//EpollReactor::data.append(".");
	//EpollReactor::data.append(to_string(((unsigned char*)data)[3]));
	//EpollReactor::data.append(":");
	//EpollReactor::data.append(to_string(((unsigned short*)data)[2]));

	EpollReactor::sdata <<
		(unsigned short)((unsigned char*)data)[0] << "." <<
		(unsigned short)((unsigned char*)data)[1] << "." <<
		(unsigned short)((unsigned char*)data)[2] << "." <<
		(unsigned short)((unsigned char*)data)[3] << ":" <<
		((unsigned short*)data)[2];
}

void EpollReactor::stdinProc(int fd, int events, void * arg)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif

	
	while (true)
	{
		char buf[64] = { 0 };
		int n = Read(fd, buf, sizeof(buf));
		if (-1 == n)
		{
			switch (errno)
			{
			case EAGAIN:
				cout << "read complete" << endl;
				goto comp;
			default:
				cout << "read err" << endl;
				return;
			}
		}
		if ('\n' == buf[n - 1]) buf[n - 1] = 0;
		cout << buf << endl;
		//data.append(buf, n);
	}
comp:
	EpollReactor* er = (EpollReactor*)arg;
	er->setEvent(STDOUT_FILENO, EPOLLOUT, stdoutProc);
}

void EpollReactor::stdoutProc(int fd, int events, void * arg)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif

	//if ('\n' == data[data.size() - 1]) 
	//	data[data.size() - 1] = 0;
	//cout << data << endl;
	
	char buf[512];
	int i = 1;

	do
	{
		memset(buf, 0, sizeof(buf));
		i = sdata.readsome(buf, sizeof(buf));
		write(STDOUT_FILENO, buf, sizeof(buf));

	}while (i);

	//data.clear();
	sdata.clear();

	EpollReactor* er = (EpollReactor*)arg;
	er->setEvent(STDOUT_FILENO, EPOLLIN | EPOLLET, stdoutProc);
}

void EpollReactor::lstnProc(int fd, int events, void * arg)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif


	sockaddr_in sa;
	socklen_t saLen = sizeof(sa);
	int connFd = Accept(fd, (sockaddr*)&sa, &saLen);

	EpollReactor* er = (EpollReactor*)arg;
	switch (connFd)
	{
	case -1:
		perror("accept err");
		break;
	default:
		int dataSize = 6;
		void* data = malloc(dataSize);
		memcpy(data, &sa.sin_addr.s_addr, 4);
		*(short*)((char*)data + 4) = ntohs(sa.sin_port);

		//EpollReactor::data.append("connected[");
		//EpollReactor::data.append(to_string(connFd));
		//EpollReactor::data.append("] : ");
		//pIP(data);
		//EpollReactor::data.append("\n");

		EpollReactor::sdata << "connected[" << connFd << "] : ";
		pIP(data);
		EpollReactor::sdata << "\n";

		int stat = fcntl(STDIN_FILENO, F_GETFL);
		fcntl(connFd, F_SETFL, stat | O_NONBLOCK);
		er->addEvent(connFd, EPOLLIN | EPOLLET, connProc, data, dataSize);
		

		er->setEvent(STDOUT_FILENO, EPOLLOUT, stdoutProc);
		break;
	}
}

void EpollReactor::connProc(int fd, int events, void * arg)
{
	#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
	#endif


	EpollReactor* er = (EpollReactor*)arg;
	while (true)
	{
		char buf[128] = { 0 };
		int n = Read(fd, buf, sizeof(buf));
		switch (n)
		{
		case -1:
			if (EAGAIN == errno) goto comp;
			er->delEvent(fd);
			return;
		case 0:
			//data.append("disconnected[");
			//data.append(to_string(fd));
			//data.append(", ");
			//data.append(to_string(er->eventMap.size()));
			//data.append("] : ");
			//pIP(er->checkEvent(fd)->data);
			//data.append("\n");

			sdata << "disconnected[" << fd << ", " << er->eventMap.size() << "] : ";
			pIP(er->checkEvent(fd)->data);
			sdata << "\n";

			er->delEvent(fd);

			goto comp;
		default:
			if ('\n' == buf[n - 1]) buf[n - 1] = 0;
			//data.append(buf, n);
			sdata << buf;
			break;
		}
	}
comp:
	er->setEvent(STDOUT_FILENO, EPOLLOUT, stdoutProc);
	return;
}

