#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <list>
#include <iostream>


#define PORT 8080

using namespace std;

void pX(void* x, int size)
{
	for (int i = 0; i < size; i++)
	{
		printf("%02X ", ((unsigned char*)x)[i]);
	}
	cout << endl;
}

struct EpollFd
{
	EpollFd(const int& fd) : fd(fd)
	{

	}
	EpollFd(const int& fd, const sockaddr_in& sa) : fd(fd)
	{
		//ipport = ((uint64_t)ntohl(sa.sin_addr.s_addr)) << 4 | (uint16_t)ntohs(sa.sin_port);
		for (int i = 0; i < 4; i++)ipport[i] = (sa.sin_addr.s_addr >> (i * 8)) & 0xff;
		uint16_t port = ntohs(sa.sin_port);
		memcpy(ipport + 4, &port, 2);
		//pX((void*)&(sa.sin_addr.s_addr), 4);
	}
	int fd;
	unsigned char ipport[6];

	void pIP()
	{
		cout << "[" << fd << "]";
		uint16_t* port = (uint16_t*)(ipport + 4);
		for (int i = 0; i < 4; i++)
		{
			cout << (uint16_t)ipport[i];
			i == 3 ? cout << ':' : cout << '.';
		}
		cout << *port << endl;
		//cout << "port " << endl;
		//pX(port, sizeof(*port));
		//cout << "ipport " << endl;
		//pX(&ipport, sizeof(ipport));

	}
	bool operator==(const EpollFd& ef)
	{
		//cout << "operator==" << endl;
		return fd == ef.fd;
	}
};

int main()
{
	epoll_event event, *events;
	//int size = 0, capacity = 32;
	list<EpollFd> listFd;
	event.events = EPOLLIN;

	int epollFd = epoll_create(32);

	event.data.fd = STDIN_FILENO;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	//listFd.push_back(STDIN_FILENO);
	//++size;

	int lstnFd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(lstnFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	event.data.fd = lstnFd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, lstnFd, &event);
	//vctrFd.push_back(lstnFd);
	//++size;

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(lstnFd, (sockaddr*)&sa, sizeof(sa));

	listen(lstnFd, 32);

	events = (epoll_event*)malloc(sizeof(event) * 64);

	event.events = EPOLLIN | EPOLLET;

	while (true)
	{
		int flag = epoll_wait(epollFd, events, 64, 5000);
		switch (flag)
		{
		case -1:
			perror("epoll_wait is failed..");
			goto err1;
		case 0:
			cout << "epoll_wait is timeout" << endl;
			break;
		default:
			cout << "flag : " << flag << endl;
			for (int i = 0; i < flag; i++)
			{
				//cout << "fd : " << events[i].data.fd << endl;
				if (STDIN_FILENO == events[i].data.fd)
				{
					char buf[64] = { 0 };
					fgets(buf, sizeof(buf) - 1, stdin);
					if ('\n' == buf[strlen(buf) - 1])
						buf[strlen(buf) - 1] = 0;
					for (EpollFd efd : listFd)
					{
						send(efd.fd, buf, strlen(buf) + 1, 0);
					}
				}
				else if (lstnFd == events[i].data.fd)
				{
					char bufIP[INET_ADDRSTRLEN];
					sockaddr_in sa;
					memset(&sa, 0, sizeof(sa));
					socklen_t saLen = sizeof(sa);
					int connFd = accept(lstnFd, (struct sockaddr*)&sa, &saLen);
					if (-1 == connFd)
					{
						perror("accept err");
						goto err1;
					}
					fcntl(connFd, F_SETFL, O_NONBLOCK);
					event.data.fd = connFd;
					epoll_ctl(epollFd, EPOLL_CTL_ADD, connFd, &event);
					EpollFd ef(connFd, sa);
					listFd.push_back(ef);
					cout << listFd.size() << endl;
					ef.pIP();
				}
				else
				{
					while (true)
					{
						char buf[4] = { 0 };
						int n = recv(events[i].data.fd, buf, sizeof(buf) - 1, 0);
						switch (n)
						{
						case -1:
							if (EAGAIN == errno) goto next;
							perror("recv err");

							listFd.remove(events[i].data.fd);
							event.data.fd = events[i].data.fd;
							epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, &event);
							close(events[i].data.fd);
							goto next;
						case 0:
							cout << "close connect" << endl;

							listFd.remove(events[i].data.fd);
							event.data.fd = events[i].data.fd;
							epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, &event);
							close(events[i].data.fd);
							goto next;
						default:
							if ('\n' == buf[n - 1])
								buf[n - 1] = 0;
							cout << buf << endl;
							break;
						}
					}
				next:
					continue;
				}
			}
			break;
		}
	}

	return 0;

err1:
	close(epollFd);
	close(lstnFd);

	return 1;
}