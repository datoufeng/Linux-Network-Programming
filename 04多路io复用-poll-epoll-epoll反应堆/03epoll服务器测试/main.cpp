#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <list>
#include <iostream>

#define PORT 8080

using namespace std;

struct EpollFd
{
	EpollFd(const int& fd, const sockaddr_in& sa) : fd(fd)
	{
		ipport = ((uint64_t)ntohl(sa.sin_addr.s_addr)) << 4 | ntohs(sa.sin_port);
	}
	int fd;
	uint64_t ipport;

	void pIP()
	{
		cout << "[" << fd << "]";
		uint32_t ip = ipport >> 4;
		uint16_t port = ipport & 0xffff;
		for (int i = 3; i >= 0; i--)
		{
			cout << ((ip >> (i * 8)) & 0xff);
			if (0 != i) cout << ".";
			else cout << ":";
		}
		cout << port << endl;
	}
};

int main()
{
	epoll_event event, *events;
	int size = 0, capacity = 32;
	list<int> listFd;
	event.events = EPOLLIN;

	int epollFd = epoll_create(32);

	event.data.fd = STDIN_FILENO;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	//listFd.push_back(STDIN_FILENO);
	++size;

	int lstnFd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(lstnFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	event.data.fd = lstnFd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, lstnFd, &event);
	//vctrFd.push_back(lstnFd);
	++size;

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(lstnFd, (sockaddr*)&sa, sizeof(sa));

	listen(lstnFd, 32);

	events = (epoll_event*)malloc(sizeof(event) * capacity);

	while (true)
	{
		int flag = epoll_wait(epollFd, events, size, 5000);
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
					for (int fd : listFd)
					{
						send(fd, buf, strlen(buf) + 1, 0);
					}
				}
				else if (lstnFd == events[i].data.fd)
				{
					cout << "lstnFd : " << lstnFd << endl;
					char bufIP[INET_ADDRSTRLEN];
					sockaddr_in sa;
					memset(&sa, 0, sizeof(sa));
					socklen_t saLen = sizeof(sa);
					int connFd = accept(lstnFd, (struct sockaddr*)&sa, &saLen);
					if (-1 == connFd) perror("accept err");
					cout << "saLen" << saLen << endl;
					cout << "connFd" << connFd << endl;
					uint32_t ip = sa.sin_addr.s_addr;
					cout << "ip" << ip << endl;
					cout <<  "port" << sa.sin_port << endl;
					listFd.push_back(connFd);
					event.data.fd = connFd;
					epoll_ctl(epollFd, EPOLL_CTL_ADD, connFd, &event);
					++size;
					inet_ntop(AF_INET, &ip, bufIP, saLen);
					EpollFd ef(connFd, sa);
					ef.pIP();
					cout << "[" << listFd.size() << "]" << bufIP << ":" << ntohs(sa.sin_port) << endl;
				}
				else
				{
					char buf[64] = { 0 };
					int n = recv(events[i].data.fd, buf, sizeof(buf) - 1, 0);
					switch (n)
					{
					case -1:
						perror("recv err");
						goto err1;
					case 0:
						cout << "close connect" << endl;
						listFd.remove(events[i].data.fd);
						event.data.fd = events[i].data.fd;
						epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, &event);
						--size;
						break;
					default:
						if ('\n' == buf[n - 1])
							buf[n - 1] = 0;
						cout << buf << endl;
						break;
					}
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