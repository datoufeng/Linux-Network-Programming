#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>

using namespace std;

int main()
{
	int listenFd = socket(AF_INET, SOCK_STREAM, 0);
	int sOpt = 1;
	setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &sOpt, sizeof(sOpt));

	sockaddr_in sain;
	int saLen = sizeof(sain);
	sain.sin_family = AF_INET;
	sain.sin_port = htons(8080);
	inet_pton(AF_INET, "192.168.233.233", &(sain.sin_addr.s_addr));

	bind(listenFd, (sockaddr*)&sain, saLen);

	listen(listenFd, 32);

	int connFd = accept(listenFd, (sockaddr*)&sain, (socklen_t*)&saLen);
	while (true)
	{
		char buf[64] = { 0 };

		int n = recv(connFd, buf, sizeof(buf), 0);
		if (0 >= n) break;
		buf[n - 1] = 0;
		cout << buf << endl;
		send(connFd, buf, sizeof(buf), 0);
	}



	return 0;
}