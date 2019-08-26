#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

using namespace std;

int main()
{
	int epollFd = epoll_create(4);
	if (-1 == epollFd)
	{
		perror("epoll_create");
		return 1;
	}
	int rdFd = open("fifo1", O_RDONLY);
	if (-1 == rdFd)
	{
		perror("open read");
		close(epollFd);
		return 1;
	}
	int wrFd = open("fifo2", O_WRONLY);
	if (-1 == wrFd)
	{
		perror("open write");
		close(rdFd);
		close(epollFd);
		return 1;
	}

	int flag;
	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = rdFd;
	flag = epoll_ctl(epollFd, EPOLL_CTL_ADD, rdFd, &event);
	if (-1 == flag)
	{
		perror("epoll_ctl rdFd");
		goto err1;
	}
	event.data.fd = STDIN_FILENO;
	flag = epoll_ctl(epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	if (-1 == flag)
	{
		perror("epoll_ctl rdFd");
		goto err1;
	}

	epoll_event events[2];
	while (true)
	{
		flag = epoll_wait(epollFd, events, 2, 2000);
		if (0 == flag)
		{
			printf("epoll is timeout\n");
			continue;
		}
		if (-1 == flag)
		{
			perror("epoll_wait is err");
			break;
		}
		for (epoll_event e : events)
		{
			if (STDIN_FILENO == e.data.fd)
			{
				char buf[128] = { 0 };
				fgets(buf, sizeof(buf), stdin);
				if ('\n' == buf[strlen(buf) - 1])
					buf[strlen(buf) - 1] = 0;
				write(wrFd, buf, strlen(buf));
			}
			else if (rdFd == e.data.fd)
			{
				char buf[128] = { 0 };
				int n = read(e.data.fd, buf, sizeof(buf) - 1);
				switch (n)
				{
				case -1:
					perror("read err");
					goto err1;
				case 0:
					printf("writeFd is close");
					goto err1;
				default:
					if ('\n' == buf[n - 1])
						buf[n - 1] = 0;
					cout << buf << endl;

					break;
				}
			}
		}
	}

	close(rdFd);
	close(wrFd);
	close(epollFd);

	return 0;
err1:
	close(rdFd);
	close(wrFd);
	close(epollFd);
	return 1;

}