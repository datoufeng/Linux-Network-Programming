#include <iostream>
#include <unistd.h>

#include "ThreadLoop.h"

using namespace std;

void proc(void* arg)
{
	//cout << (char*)arg << endl;
	int i = rand() % 5;
	cout << "delay : " << i << endl;
	sleep(i);
}


int main()
{
	srand(time(NULL));

	ThreadLoop thrLp;
	for (int i = 0; i < 15; i++)
		thrLp.addTask(proc, (void*)"hello");
	cout << "add complete" << endl;

	while (true)
	{
		sleep(3);
		//pthread_cond_signal(&thrLp.thrInfo->noEmptyThrLoop);
	}

	return 0;
}