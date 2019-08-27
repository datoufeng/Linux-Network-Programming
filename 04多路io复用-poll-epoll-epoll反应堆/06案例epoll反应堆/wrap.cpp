#include <iostream>
#include <string.h>
#include <unistd.h>

#include "wrap.h"

using namespace std;

void perr_exit(const char * s)
{
	perror(s);
	exit(1);
}

int Accept(int fd, sockaddr * sa, socklen_t * salenptr)
{
	int connFd;
again:
	if (0 > (connFd = accept(fd, sa, salenptr)))
	{
		//如果accept被信号打断时错误码为EINTR
		//或者重置位RES置1时，错误码为ECONNABORTED
		//那么将继续监听
		if (ECONNABORTED == errno || EINTR == errno)
			goto again;
		else
			perr_exit("accept error");
	}
	return connFd;
}

int Bind(int fd, const sockaddr * sa, socklen_t salen)
{
	int ret = bind(fd, sa, salen);
	if (0 > ret)perr_exit("bind error");
	return ret;
}

int Connect(int fd, const sockaddr * sa, socklen_t salen)
{
	int ret = connect(fd, sa, salen);
	if (0 > ret)perr_exit("connect error");
	return ret;
}

int Listen(int fd, int backlog)
{
	int ret = listen(fd, backlog);
	if (0 > ret)perr_exit("listen error");
	return ret;
}

int Socket(int family, int type, int protocol)
{
	int ret = socket(family, type, protocol);
	if (0 > ret)perr_exit("socket error");
	return ret;
}

int Read(int fd, void * ptr, size_t nbytes)
{
	int ret;
again:

	if (0 > (ret = read(fd, ptr, nbytes)))
	{
		if (EINTR == errno) goto again;
		else return -1;
	}
	return ret;
}

int Write(int fd, const void * ptr, unsigned int nbytes)
{
	int ret;
again:

	if (0 > (ret = write(fd, ptr, nbytes)))
	{
		if (EINTR == errno) goto again;
		else return -1;
	}
	return ret;
}

int Close(int fd)
{
	int ret = close(fd);
	if (0 > ret)perr_exit("close error");
	return ret;
}

int Readn(int fd, void * vptr, unsigned int n)
{
	int remainN = n;
	char* ptr = (char*)vptr;
	
	while (0 < remainN)
	{
		int readN;
		if (0 > (readN = read(fd, ptr, remainN)))
		{
			if (EINTR == errno) readN = 0;
			else return -1;
		}
		else if (0 == readN)break;
		remainN -= readN;
		ptr += readN;
	}

	return n - remainN;
}

int Writen(int fd, const void * vptr, unsigned int n)
{
	int remainN = n;
	char* ptr = (char*)vptr;

	while (0 < remainN)
	{
		int writeN;
		if (0 > (writeN = write(fd, ptr, remainN)))
		{
			if (EINTR == errno) writeN = 0;
			else return -1;
		}
		else if (0 == writeN) return -1;
		remainN -= writeN;
		ptr += writeN;
	}

	return n;
}

int Readline(int fd, void * vptr, unsigned int maxlen)
{
	int lineN = 0;
	memset(vptr, 0, maxlen);
	for (int i = 0; i < maxlen - 1; i++)
	{
		int readN;
		if (0 > (readN = Readn(fd, (char*)vptr + i, 1))) return -1;
		else if (0 == readN) break;
		else
		{
			if ('\n' == ((char*)vptr)[i])
			{
				((char*)vptr)[i] = '\0';
				break;
			}
			lineN += 1;
		}
	}

	return lineN;
}



