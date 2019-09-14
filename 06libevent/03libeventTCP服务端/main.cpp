#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <event.h>

using namespace std;

#define PORT 8080

void conProc(evutil_socket_t cfd, short what, void* arg)
{
	event** p_conEvent = (event**)arg;

	if (what & EV_TIMEOUT)
	{
		cout << "timeout: disconnected.." << endl;
		event_del(*p_conEvent);
		event_free(*p_conEvent);
		free(p_conEvent);
		close(cfd);
		return;
	}

	char buf[128] = { 0 };

	switch (recv(cfd, buf, sizeof(buf), 0))
	{
	case -1:
		perror("recv err..");
		event_del(*p_conEvent);
		event_free(*p_conEvent);
		free(p_conEvent);
		close(cfd);
		return;
	case 0:
		cout << "connect closed.." << endl;
		event_del(*p_conEvent);
		event_free(*p_conEvent);
		free(p_conEvent);
		close(cfd);
		return;
	default:
		if ('\n' == buf[strlen(buf) - 1])
			buf[strlen(buf) - 1] = '\0';
		cout << buf << endl;
		break;
	}
}

void ltnProc(evutil_socket_t lfd, short what, void* arg)
{
	event_base* eb = (event_base*)arg;
	timeval timeout = { 5, 0 };

	sockaddr_in sa;
	socklen_t sockLen = sizeof(sa);
	int conFd = accept(lfd, (sockaddr*)&sa, &sockLen);

	char strIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &sa, strIP, INET_ADDRSTRLEN);
	printf("[%s:%u]connected..\n", strIP, ntohs(sa.sin_port));

	event** p_conEvent = (event**)malloc(sizeof(event*));
	*p_conEvent = event_new(eb, conFd, EV_READ | EV_ET | EV_PERSIST, conProc, p_conEvent);

	event_add(*p_conEvent, &timeout);
}

int main()
{
	int ltnFd = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
	setsockopt(ltnFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(ltnFd, (sockaddr*)&sa, sizeof(sa));

	listen(ltnFd, 128);

	event_base* eb = event_base_new();

	event* ltnEvent = event_new(eb, ltnFd, EV_READ | EV_PERSIST, ltnProc, eb);
	event_add(ltnEvent, NULL);

	event_base_dispatch(eb);
	event_free(ltnEvent);
	event_base_free(eb);

	return 0;
}