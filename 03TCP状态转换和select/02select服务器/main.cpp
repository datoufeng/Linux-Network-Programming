#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>

using namespace std;

pthread_rwlock_t rwSelLock;

fd_set readFds;
int maxConnFd = 0;

void* selThdProc(void*)
{
	timeval timer = { 0, 0 };
	while (1)
	{
		pthread_rwlock_rdlock(&rwSelLock);
		fd_set readFdsTemp = readFds;
		int fdsTemp = maxConnFd + 1;
		pthread_rwlock_unlock(&rwSelLock);
		int n = select(fdsTemp, &readFdsTemp, NULL, NULL, &timer);
		//cout << "select" << endl;
		//int n = select(fdsTemp, &readFdsTemp, NULL, NULL, NULL);
		//cout << n << endl;
		if (0 > n)break;
		else
		{
			while(n)
			{
				char buf[64] = { 0 };
				for (int i = 0; i <= maxConnFd; i++)
				{
					int flag = FD_ISSET(i, &readFdsTemp);
					if (1 == flag)
					{
						int left = recv(i, buf, sizeof(buf), 0);
						buf[left - 1] = 0;
						cout << buf << endl;
						if (--n) break;
					}

				}
			}
		}
	}
}

int main()
{
	int lstnFd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(lstnFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in saIn;
	int sockLen = sizeof(saIn);

	saIn.sin_family = AF_INET;
	saIn.sin_port = htons(8080);
	inet_pton(AF_INET, "192.168.233.233", &(saIn.sin_addr.s_addr));

	bind(lstnFd, (sockaddr*)&saIn, sockLen);

	listen(lstnFd, 64);

	int connFd;
	FD_ZERO(&readFds);
	FD_SET(lstnFd, &readFds);

	pthread_rwlock_init(&rwSelLock, NULL);

	pthread_t selectThread;
	pthread_attr_t selThdAttr;
	pthread_attr_init(&selThdAttr);
	pthread_attr_setdetachstate(&selThdAttr, PTHREAD_CREATE_DETACHED);
	pthread_create(&selectThread, &selThdAttr, selThdProc, NULL);
	pthread_attr_destroy(&selThdAttr);
	while (1)
	{
		connFd = accept(lstnFd, (sockaddr*)&saIn, (socklen_t*)&sockLen);
		if (0 >= connFd)break;
		
		if (maxConnFd < connFd) maxConnFd = connFd;

		pthread_rwlock_wrlock(&rwSelLock);
		FD_SET(connFd, &readFds);
		pthread_rwlock_unlock(&rwSelLock);
	}


	return 0;
}