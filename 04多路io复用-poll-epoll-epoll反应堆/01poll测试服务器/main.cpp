#include <poll.h>
#include <iostream>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080

int main()
{
	pollfd* fd = (pollfd*)malloc(32 * sizeof(pollfd));
	int index = -1;

	int lstnFd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(lstnFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(lstnFd, (sockaddr*)&sa, sizeof(sa));
	listen(lstnFd, 32);

	fd[++index] = { lstnFd, POLLIN, 0 };

	while (true)
	{
		int n = poll(fd, index + 1, -1);
		for (int i = 0; i <= index; i++)
		{
			if (-1 == fd[i].fd) continue;
			if (POLLIN != fd[i].revents) continue;
			if (lstnFd == fd[i].fd)
			{
				sockaddr_in sa;
				int saLen;
				char buf[INET_ADDRSTRLEN];
				int connFd = accept(lstnFd, (sockaddr*)&sa, (socklen_t*)&saLen);
				inet_ntop(AF_INET, &sa, buf, sizeof(sa));
				cout << "connected to : " << buf << ":" << ntohs(sa.sin_port) << endl;
				fd[++index] = { connFd, POLLIN, 0 };
			}
			else 
			{
				int readN = -1;
				char buf[128] = { 0 };
				switch (readN = recv(fd[i].fd, buf, sizeof(buf), 0))
				{
				case -1:
					perror("recv err");
					break;
				case 0:
					fd[i] = fd[index];
					fd[index].fd = -1;
					--index;
					break;
				default:
					buf[readN - 1] = 0;
					cout << buf << endl;
					break;
				}
			}
		}
		

	}

	return 0;
}