#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <event.h>

#define FIFO "event.fifo"

void eventProc(evutil_socket_t fd, short what,void* arg)
{
	printf("%s fd[%d],event : %s%s%s%s\n", (char*)arg, fd,
		what & EV_TIMEOUT ? "timeout" : "",
		what & EV_READ ? "read" : "",
		what & EV_WRITE ? "write" : "",
		what & EV_SIGNAL ? "signal" : "");
}

void eventLoop(int fd1, int fd2)
{
	event_base* eb = event_base_new();

	timeval timeout = { 5, 0 };

	event* e1 = event_new(eb, fd1, EV_READ | EV_PERSIST, eventProc, (void*)"readEvent");
	event* e2 = event_new(eb, fd2, EV_WRITE | EV_ET | EV_PERSIST, eventProc, (void*)"writeEvent");

	event_add(e1, &timeout);
	event_add(e2, NULL);

	event_base_dispatch(eb);
	event_base_free(eb);
}

int main()
{
	unlink(FIFO);
	mkfifo(FIFO, 0644);

	int fd1 = open(FIFO, O_RDONLY | O_NONBLOCK, 0644);
	int fd2 = open(FIFO, O_WRONLY | O_NONBLOCK, 0644);

	eventLoop(fd1, fd2);

	close(fd1);
	close(fd2);

	return 0;
}