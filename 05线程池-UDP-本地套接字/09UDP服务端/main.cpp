#include <iostream>
#include <string.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080

int main()
{
	int udpFd = socket(AF_INET, SOCK_DGRAM, 0);
	int opt = 0;
	setsockopt(udpFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = INADDR_ANY;
	bind(udpFd, (sockaddr*)&sa, sizeof(sa));

	while (true)
	{
		char buf[128] = { 0 };

		socklen_t saLen = sizeof(sa);
		int n = recvfrom(udpFd, buf, sizeof(buf), 0, (sockaddr*)&sa, &saLen);

		cout << "[" << n << "]" << buf << endl;

		memset(buf, 0, sizeof(buf));
		cin >> buf;
		sendto(udpFd, buf, strlen(buf), 0, (sockaddr*)&sa, sizeof(sa));
	}


	return 0;
}