#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

using namespace std;

#define SERVER_PORT 8080
#define CLIENT_PORT 9090
#define GROUP_ADDR "224.0.2.0"

int main()
{
	int udpFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == udpFd)
	{
		perror("socket");
		return udpFd;
	}

	sockaddr_in serverSa;
	serverSa.sin_family = AF_INET;
	serverSa.sin_addr.s_addr = htonl(INADDR_ANY);
	serverSa.sin_port = htons(SERVER_PORT);

	int ret = bind(udpFd, (sockaddr*)&serverSa, sizeof(serverSa));
	if (-1 == ret)
	{
		perror("bind");
		return ret;
	}
	
	sockaddr_in clientSa;
	clientSa.sin_family = AF_INET;
	//clientSa.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, GROUP_ADDR, &clientSa.sin_addr.s_addr);
	clientSa.sin_port = htons(CLIENT_PORT);

	while (true)
	{
		sendto(udpFd, "group", 7, 0, (sockaddr*)&clientSa, sizeof(clientSa));
		sleep(1);
	}

	return 0;
}