#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <event.h>
#include <event2/listener.h>

using namespace std;

#define PORT 8080


void readcbProc(bufferevent* bufEv, void* arg)
{
	//cout << __FUNCTION__ << endl;

	char buf[128] = { 0 };
	size_t n = bufferevent_read(bufEv, buf, sizeof(buf) - 1);
	if ('\n' == buf[n - 1])buf[n - 1] = '\0';
	cout << buf << endl;
	
}

//void writecbProc(bufferevent* bufEv, void* arg)
//{
//	cout << __FUNCTION__ << endl;
//}

void eventcbProc(bufferevent* bufEv, short what, void* arg)
{
	if (what & BEV_EVENT_ERROR)
	{
		printf("connection error...\n");
		bufferevent_free(bufEv);
		timeval timeout = { 2, 0 };
		event_base_loopexit((event_base*)arg, &timeout);
	}
	else if (what & BEV_EVENT_EOF)
	{
		printf("connection closed...\n");
		bufferevent_free(bufEv);
	}

	//printf("event[%d] : %s%s%s%s%s%s\n", what,
	//	what & BEV_EVENT_EOF ? "eof " : "",
	//	what & BEV_EVENT_ERROR ? "error " : "",
	//	what & BEV_EVENT_CONNECTED ? "connected " : "",
	//	what & BEV_EVENT_READING ? "reading " : "",
	//	what & BEV_EVENT_WRITING ? "writing " : "",
	//	what & BEV_EVENT_TIMEOUT ? "timeout " : "");
}

void ltnrcbProc(evconnlistener* cl, evutil_socket_t cfd, sockaddr* sa, int sockLen, void* arg)
{
	bufferevent* be = bufferevent_socket_new((event_base*)arg, cfd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(be, readcbProc, NULL, eventcbProc, arg);
	bufferevent_enable(be, EV_READ /*| EV_WRITE*/);
	bufferevent_disable(be, EV_WRITE);

	char strIp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &((sockaddr_in*)sa)->sin_addr.s_addr, strIp, INET_ADDRSTRLEN);
	printf("connected[%s:%d]\n", strIp, ntohs(((sockaddr_in*)sa)->sin_port));

}

void intcbProc(evutil_socket_t sig, short what, void* arg)
{
	void** sigEb = (void**)arg;
	event *intSig = (event*)sigEb[0];
	event_base* eb = (event_base*)sigEb[1];

	//printf("sig[%d],event : %s%s%s%s\n", sig,
	//	what & EV_TIMEOUT ? "timeout" : "",
	//	what & EV_READ ? "read" : "",
	//	what & EV_WRITE ? "write" : "",
	//	what & EV_SIGNAL ? "signal" : "");

	printf("proc is closing..delay 2 sec..\n");
	evsignal_del(intSig);
	event_free(intSig);
	timeval timeout = { 2, 0 };
	event_base_loopexit(eb, &timeout);
}

int main()
{
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	event_base* eb = event_base_new();

	evconnlistener* evLtnr = 
		evconnlistener_new_bind(eb, ltnrcbProc, eb, 
			LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, 
			(sockaddr*)&sa, sizeof(sa));

	void* sigEb[2];
	sigEb[1] = eb;
	event* intSig = evsignal_new(eb, SIGINT, intcbProc, sigEb);
	sigEb[0] = intSig;
	event_add(intSig, NULL);

	event_base_dispatch(eb);
	evconnlistener_free(evLtnr);
	event_base_free(eb);
	return 0;
}