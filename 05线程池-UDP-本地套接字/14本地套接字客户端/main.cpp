#include <iostream>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/un.h>

using namespace std;

int main()
{
	int conFd = socket(AF_LOCAL, SOCK_STREAM, 0);

	unlink("client.socket");

	sockaddr_un sa;
	sa.sun_family = AF_LOCAL;
	strcpy(sa.sun_path, "client.socket");
	int ret = bind(conFd, (sockaddr*)&sa, sizeof(sa));
	if (-1 == ret)
	{
		perror("bind err");
		return -1;
	}
	
	strcpy(sa.sun_path, "server.socket");
	ret = connect(conFd, (sockaddr*)&sa, sizeof(sa));
	if (-1 == ret)
	{
		perror("connect err");
		return -1;
	}

	while (true)
	{
		char buf[128] = { 0 };
		cin >> buf;
		send(conFd, buf, strlen(buf), 0);
		memset(buf, 0, sizeof(buf));
		ret = recv(conFd, buf, sizeof(buf), 0);
		if (-1 == ret)
		{
			perror("recv err");
			break;
		}
		cout << buf << endl;
	}

	close(conFd);
	return 0;
}