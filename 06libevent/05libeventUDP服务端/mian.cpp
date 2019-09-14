#include <iostream>
#include <unistd.h>
#include <event.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080

void udpProc(evutil_socket_t udpFd, short what, void* arg)
{
	event** p_udpEvent = (event**)arg;

	sockaddr_in sa;
	socklen_t sockLen = sizeof(sa);
	char buf[128] = { 0 };

	//int n;
	switch (int n = recvfrom(udpFd, buf, sizeof(buf), 0, (sockaddr*)&sa, &sockLen))
	{
	case -1:
		perror("recv err..");
		event_del(*p_udpEvent);
		event_free(*p_udpEvent);
		free(p_udpEvent);
		return;
	case 0:
		printf("connection closed..");
		event_del(*p_udpEvent);
		event_free(*p_udpEvent);
		free(p_udpEvent);
		return;
	default:
		if ('\n' == buf[n - 1])
			buf[n - 1] = '\0';
		char strIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &sa, strIP, INET_ADDRSTRLEN);
		cout << "[" << strIP << ":" << ntohs(sa.sin_port) << "]";
		cout << buf << endl;
		break;
	} 
}

int main()
{
	int udpFd = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(udpFd, (sockaddr*)&sa, sizeof(sa));

	event_base* eb = event_base_new();
	
	event** p_udpEvent = (event**)malloc(sizeof(event*));
	*p_udpEvent = event_new(eb, udpFd, EV_READ | EV_PERSIST, udpProc, p_udpEvent);
	event_add(*p_udpEvent, NULL);
	event_base_dispatch(eb);
	
	event_base_free(eb);

	return 0;
}
