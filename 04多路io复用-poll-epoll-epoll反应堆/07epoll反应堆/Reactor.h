#pragma once
class Reactor
{
	struct __Event
	{
		int fdl;
		int events;
		void(*callback)(int fd, int events, void* arg);
	};
public:
	Reactor();
	~Reactor();

private:
	int epollFd;
};

