#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <list>
#include <iostream>

#define PORT 8080

using namespace std;

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
			for (int i = 0; i < flag; i++)
			{
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
					char bufIP[INET_ADDRSTRLEN];
					sockaddr_in sa;
					socklen_t saLen;
					int connFd = accept(lstnFd, (sockaddr*)&sa, &saLen);
					listFd.push_back(connFd);
					event.data.fd = connFd;
					epoll_ctl(epollFd, EPOLL_CTL_ADD, connFd, &event);
					++size;
					inet_ntop(AF_INET, &sa, bufIP, saLen);
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