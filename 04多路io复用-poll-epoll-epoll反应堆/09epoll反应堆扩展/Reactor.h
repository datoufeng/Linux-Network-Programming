#pragma once
#include <map>
#include <sstream>

#define CALLBACK static

constexpr int MAX_EVENTS = 1024;
constexpr int TIMEOUT = 10;
constexpr int NO_TIMEOUT = -1;

enum EPOLL_EVENT_STATE
{
	//EES_DELETE,
	EES_DEFAULT,
	EES_NOTIMEOUT
};

struct epoll_event;
class EpollReactor
{
	struct __Event
	{
		int fd;
		int events;
		void(*callback)(int fd, int events, void* arg);
		void* data;
		int dataSize;
		int state;
		unsigned long lastAction;
	};

public:
	EpollReactor(
		const int& size, 
		const int& maxevents = MAX_EVENTS,
		const int& timeout = NO_TIMEOUT);
	~EpollReactor();

	__Event* checkEvent(const int& fd); 
	void run();

	int listen(const int& port);

	int addEvent(
		const int& fd, const int& events,
		void(*const& callback)(int fd, int events, void* arg),
		const int& state = EES_DEFAULT,
		void* const & data = NULL, const int& dataSize = 0);
	int delEvent(const int& fd);
	int setEvent(
		const int& fd,
		const int& events,
		void(*const& callback)(int fd, int events, void* arg),
		const int& state = EES_DEFAULT,
		void* const & data = NULL, const int& dataSize = 0);
private:
	int epollFd;
	epoll_event* events;
	int maxevents, timeout;
	static std::stringstream sdata;
	//static std::string data;

	std::map<int, __Event> eventMap;
	std::multimap<unsigned long, __Event*> tcpEventMap;

	CALLBACK void pIP(void* data);

	CALLBACK void stdinProc(int fd, int events, void* arg);
	CALLBACK void stdoutProc(int fd, int events, void* arg);
	CALLBACK void timerProc(int fd, int events, void* arg);
	CALLBACK void lstnProc(int fd, int events, void* arg);
	CALLBACK void connProc(int fd, int events, void* arg);
};

