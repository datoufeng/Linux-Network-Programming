#include <iostream>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/un.h>


using namespace std;

int main()
{
	int ltnFd = socket(AF_LOCAL, SOCK_STREAM, 0);

	unlink("server.socket");

	sockaddr_un sa;
	sa.sun_family = AF_LOCAL;
	strcpy(sa.sun_path, "server.socket");

	bind(ltnFd, (sockaddr*)&sa, sizeof(sa));

	listen(ltnFd, 128);

	socklen_t sockLen = sizeof(sa);
	int conFd = accept(ltnFd, (sockaddr*)&sa, &sockLen);

	while (true)
	{
		char buf[128] = { 0 };
		int ret = recv(conFd, buf, sizeof(buf), 0);
		if (-1 == ret)
		{
			perror("recv err");
			break;
		}
		cout << buf << endl;
		memset(buf, 0, sizeof(buf));
		cin >> buf;
		send(conFd, buf, strlen(buf), 0);
	}
	
	close(ltnFd);
	close(conFd);

	return 0;
}