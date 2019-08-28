#include <sys/epoll.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "Reactor.h"

using namespace std;

EpollReactor::EpollReactor(const int& size)
{
	cout << __FUNCTION__ << endl;
	epollFd = epoll_create(size);
}


EpollReactor::~EpollReactor()
{
	cout << __FUNCTION__ << endl;
	close(epollFd);
}

EpollReactor::__Event * EpollReactor::checkEvent(const int & fd)
{
	cout << __FUNCTION__ << endl;
	std::map<int, __Event>::iterator it;
	if (eventMap.end() == (it = eventMap.find(fd)))return nullptr;
	return &it->second;
}


int EpollReactor::addEvent(const int & fd, const int & events, void(*const & callback)(int fd, int events, void *arg))
{
	cout << __FUNCTION__ << endl;
	if (eventMap.end() != eventMap.find(fd) || NULL == callback)
		return -1;

	eventMap[fd] = { fd, events, callback, NULL, 0 };

	epoll_event ee;
	ee.events = events;
	ee.data.ptr = &eventMap[fd];

	epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ee);


	return 0;
}

int EpollReactor::delEvent(const int & fd)
{
	cout << __FUNCTION__ << endl;
	std::map<int, __Event>::iterator it;
	if (eventMap.end() == (it = eventMap.find(fd))) return -1;

	epoll_event ee;
	ee.data.ptr = NULL;
	ee.events = it->second.events;

	epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ee);

	eventMap.erase(it);

	return 0;
}

int EpollReactor::setEvent(const int & fd, const int & events, void(*const & callback)(int fd, int events, void *arg))
{
	cout << __FUNCTION__ << endl;
	std::map<int, __Event>::iterator it;
	if (eventMap.end() == (it = eventMap.find(fd)) || NULL == callback)
		return -1;
	
	it->second.events = events;
	it->second.callback = callback;

	epoll_event ee;
	ee.events = events;
	ee.data.ptr = &eventMap[fd];

	epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ee);

	return 0;
}

