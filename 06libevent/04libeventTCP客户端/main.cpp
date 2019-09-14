#include <iostream>
#include <string.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080

int main()
{
	int conFd = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	connect(conFd, (sockaddr*)&sa, sizeof(sa));

	char buf[128] = { 0 };
	while (true)
	{
		memset(buf, 0, sizeof(buf));
		cin >> buf;
		send(conFd, buf, strlen(buf), 0);
	}

	return 0;
}