#include <iostream>
#include <string.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080
#define INADDR "192.168.233.233"

int main()
{
	int udpFd = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	inet_pton(AF_INET, INADDR, &sa.sin_addr.s_addr);

	while (true)
	{
		char buf[128] = { 0 };
		cin >> buf;

		sendto(udpFd, buf, strlen(buf), 0, (sockaddr*)&sa, sizeof(sa));

		memset(buf, 0, sizeof(buf));
		socklen_t sockLen = sizeof(sa);
		recvfrom(udpFd, buf, sizeof(buf), 0, (sockaddr*)&sa, &sockLen);

		cout << buf << endl;

	}


	return 0;
}