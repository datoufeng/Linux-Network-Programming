#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <signal.h>

using namespace std;

#define PORT 8080

int main()
{
	int clientFD[FD_SETSIZE];
	memset(clientFD, -1, sizeof(clientFD));
	int index = -1;
	fd_set readFds;
	FD_ZERO(&readFds);
	int maxFD = -1;

	int lstnFd = socket(AF_INET, SOCK_STREAM, 0);
	clientFD[++index] = lstnFd;
	if (maxFD < lstnFd) maxFD = lstnFd;
	int opt = 1;
	setsockopt(lstnFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	FD_SET(lstnFd, &readFds);

	sockaddr_in saIn;
	int sockLen = sizeof(saIn);
	saIn.sin_family = AF_INET;
	saIn.sin_port = htons(PORT);
	saIn.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(lstnFd, (sockaddr*)&saIn, sockLen);
	listen(lstnFd, 64);

	sigset_t sigSet;
	sigemptyset(&sigSet);
	sigaddset(&sigSet, SIGINT);

	while (true)
	{
		fd_set readFdsTmp = readFds;
		int n = pselect(maxFD + 1, &readFdsTmp, NULL, NULL, NULL, &sigSet);

		if (FD_ISSET(lstnFd, &readFdsTmp))
		{
			char buf[INET_ADDRSTRLEN];
			int connFd = accept(lstnFd, (sockaddr*)&saIn, (socklen_t*)&sockLen);
			inet_ntop(AF_INET, &saIn, buf, sockLen);
			cout << "connection established : " << buf << ":" << ntohs(saIn.sin_port) << endl;
			clientFD[++index] = connFd;
			FD_SET(connFd, &readFds);
			if (maxFD < connFd) maxFD = connFd;

			continue;
		}
		else
		{
			for (int i = 0; i <= index; i++)
			{
				char buf[128] = { 0 };
				if (-1 == clientFD[i]) break;
				if (FD_ISSET(clientFD[i], &readFdsTmp))
				{
					int n;
					switch (n = recv(clientFD[i], buf, sizeof(buf), 0))
					{
					case -1:
						perror("recv err");
						break;
					case 0:
						FD_CLR(clientFD[i], &readFds);
						if (i != index)
							clientFD[i] = clientFD[index];
						else
							clientFD[i] = -1;
						--index;
						break;
					default:
						buf[n - 1] = 0;
						cout << buf << endl;
						break;
					}
				}
			}
		}
	}


	return 0;
}