#pragma once
#include <map>

class EpollReactor
{
	struct __Event
	{
		int fd;
		int events;
		void(*callback)(int fd, int events, void* arg);
		void* data;
		int dataSize;
	};
public:
	EpollReactor(const int& size);
	~EpollReactor();

	__Event* checkEvent(const int& fd); 

	int addEvent(
		const int& fd,
		const int& events,
		void(*const& callback)(int fd, int events, void* arg));
	int delEvent(const int& fd);
	int setEvent(
		const int& fd,
		const int& events,
		void(*const& callback)(int fd, int events, void* arg));
private:
	int epollFd;
	std::map<int, __Event> eventMap;

	
};

