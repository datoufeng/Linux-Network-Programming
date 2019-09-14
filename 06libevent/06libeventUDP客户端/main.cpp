#include <iostream>
#include <unistd.h>
#include <string.h>
#include <event.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080

int main()
{
	int udpFd = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	while (true)
	{
		char buf[128] = { 0 };
		cin >> buf;
		sendto(udpFd, buf, strlen(buf) + 1, 0, (sockaddr*)&sa, sizeof(sa));
	}

	return 0;
}
