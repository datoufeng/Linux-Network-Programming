#include <iostream>
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

	sockaddr_in clientSa;
	clientSa.sin_family = AF_INET;
	clientSa.sin_addr.s_addr = htonl(INADDR_ANY);
	clientSa.sin_port = htons(CLIENT_PORT);

	int ret = bind(udpFd, (sockaddr*)&clientSa, sizeof(clientSa));
	if (-1 == ret)
	{
		perror("bind");
		return ret;
	}

	int opt = 1;
	setsockopt(udpFd, IPPROTO_IP, IPV6_MULTICAST_LOOP, &opt, sizeof(opt));

	ip_mreq group;

	inet_pton(AF_INET, GROUP_ADDR, &group.imr_multiaddr.s_addr);
	group.imr_interface.s_addr = ntohl(INADDR_ANY);

	setsockopt(udpFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group));
	

	int i = 0;

	while (true)
	{
		char buf[128] = { 0 };
		//cout << "recvfrom" << endl;
		recvfrom(udpFd, buf, sizeof(buf), 0, NULL, NULL);

		cout << buf << endl;

		if (5 == i++)
			setsockopt(udpFd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &group, sizeof(group));
	}



	return 0;
}