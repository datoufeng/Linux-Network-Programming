#include <iostream>
#include <string.h>
#include <unistd.h>

#include "ThreadLoop.h"

using namespace std;

void proc(pthread_t thrId, TaskId id, void* arg)
{
	//cout << (char*)arg << endl;
	int i = rand() % 5;
	
	char buf[128] = { 0 };
	sprintf(buf, "[%u]task id[%u], delay[%u]\n", thrId, id, i);
	write(STDOUT_FILENO, buf, strlen(buf));
	sleep(i);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "\t\t\ttask finish [%u]\n", id);
	write(STDOUT_FILENO, buf, strlen(buf));
}


int main()
{
	srand(time(NULL));

	ThreadLoop thrLp;
	for (int i = 0; i < 30; i++)
		thrLp.addTask(proc, (void*)"hello");
	cout << "add complete" << endl;

	while (true)
	{
		sleep(3);
		//pthread_cond_signal(&thrLp.thrInfo->noEmptyThrLoop);
	}

	return 0;
}