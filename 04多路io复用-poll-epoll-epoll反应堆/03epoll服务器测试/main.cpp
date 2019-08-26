#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <iostream>

#define PORT 8080

using namespace std;

int main()
{
	epoll_event event, *events;
	int size = 0, capacity = 32;
	vector<int> vctrFd;
	event.events = EPOLLIN;

	int epollFd = epoll_create(32);

	event.data.fd = STDIN_FILENO;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	++size;

	int lstnFd = socket(AF_INET, SOCK_STREAM, 0);

	event.data.fd = lstnFd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, lstnFd, &event);
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
					for (int fd : vctrFd)
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
					inet_ntop(AF_INET, &sa, bufIP, saLen);
					cout << bufIP << ":" << ntohs(sa.sin_port) << endl;
					vctrFd.push_back(connFd);
				}
				else
				{

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