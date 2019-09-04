#include "ThreadLoop.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>

//#define __DEBUG
#define __DEBUGP(X) cout << #X" : " << X << endl

using namespace std;

//ThreadLoop::TaskId ThreadLoop::taskId = 0;
const int ThreadLoop::defaultThrNum = 5;
const int ThreadLoop::defaultMaxThrNum = 20;

ThreadLoop::ThreadLoop(const int &thrNum, const int & maxTskNum)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG
	

	thrInfo = (__ThreadInfo*)malloc(sizeof(__ThreadInfo));

	if (thrNum <= 0 || maxTskNum <= 0)
	{
		thrInfo->thrNum = defaultThrNum;
		thrInfo->maxTskNum = defaultMaxThrNum;
	}
	else if (thrNum > maxTskNum)
	{
		thrInfo->thrNum = maxTskNum;
		thrInfo->maxTskNum = maxTskNum;
	}
	else
	{
		thrInfo->thrNum = thrNum;
		thrInfo->maxTskNum = maxTskNum;
	}

	thrInfo->thrArr = (pthread_t*)malloc(sizeof(pthread_t) * thrInfo->thrNum);
	thrInfo->taskArr = (__Task**)malloc(sizeof(__Task*) * thrInfo->maxTskNum);
	memset(thrInfo->thrArr, 0, sizeof(pthread_t) * thrInfo->thrNum);
	memset(thrInfo->taskArr, 0, sizeof(__Task*) * thrInfo->maxTskNum);

	thrInfo->isClose = 0;
	thrInfo->jobWaiting = 0;
	
	//thrInfo->jobThrNum = 0;

	thrInfo->taskNum = 0;
	thrInfo->taskBeg = 0;

	pthread_attr_t thrAttr;
	pthread_attr_init(&thrAttr);
	pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);
	
	pthread_mutex_init(&thrInfo->thrMutex, NULL);
	pthread_mutex_init(&thrInfo->condMutex, NULL);
	pthread_cond_init(&thrInfo->emptyThrLoop, NULL);
	pthread_cond_init(&thrInfo->noEmptyThrLoop, NULL);

	for (int i = 0; i < thrInfo->thrNum; i++)
	{
		pthread_create(thrInfo->thrArr + i, &thrAttr, thrProc, thrInfo);
	}
}


ThreadLoop::~ThreadLoop()
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	pthread_cond_destroy(&thrInfo->noEmptyThrLoop);
	pthread_cond_destroy(&thrInfo->emptyThrLoop);
	pthread_mutex_destroy(&thrInfo->thrMutex);
	pthread_mutex_destroy(&thrInfo->condMutex);

	free(thrInfo->taskArr);
	free(thrInfo->thrArr);
	free(thrInfo);
}

void ThreadLoop::addTask(void (*const & thrProc)(pthread_t, TaskId, void *), void* const& arg)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	while (thrInfo->maxTskNum <= thrInfo->taskNum)
	{
		pthread_cond_wait(&thrInfo->emptyThrLoop, &thrInfo->condMutex);
	}
	__Task* task = (__Task*)malloc(sizeof(__Task));
	task->id = taskId++;
	task->proc = thrProc;
	task->arg = arg;

	pthread_mutex_lock(&thrInfo->thrMutex);

	int pos = (thrInfo->taskBeg + thrInfo->taskNum) % thrInfo->maxTskNum;
	thrInfo->taskArr[pos] = task;
	++thrInfo->taskNum;

	pthread_mutex_unlock(&thrInfo->thrMutex);

	if (0 < thrInfo->jobWaiting && 0 < thrInfo->taskNum)
		pthread_cond_signal(&thrInfo->noEmptyThrLoop);
}

void ThreadLoop::delTask(void (*const & thrProc)(void *), void* const& arg)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

}

void * ThreadLoop::thrProc(void * arg)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	//cout << pthread_self() << endl;
	
	__ThreadInfo* thrInfo = (__ThreadInfo*)arg;

	pthread_mutex_lock(&thrInfo->thrMutex);
	++thrInfo->jobWaiting;
	pthread_mutex_unlock(&thrInfo->thrMutex);

	while (true)
	{
		__Task* task;
		while (0 >= thrInfo->taskNum && !thrInfo->isClose)
		{
			char buf[128] = { 0 };
			sprintf(buf, "[%u]%u\n", pthread_self(), thrInfo->taskNum);
			write(STDOUT_FILENO, buf, strlen(buf));
			pthread_cond_wait(&thrInfo->noEmptyThrLoop, &thrInfo->condMutex);
		}
		pthread_mutex_lock(&thrInfo->thrMutex);

		--thrInfo->jobWaiting;
		if (thrInfo->taskNum)
		{
			task = thrInfo->taskArr[thrInfo->taskBeg];
			thrInfo->taskArr[thrInfo->taskBeg] = 0;
			thrInfo->taskBeg =
				(thrInfo->maxTskNum + thrInfo->taskBeg + 1) % 
				thrInfo->maxTskNum;
			--thrInfo->taskNum;
		}
		if (thrInfo->isClose)
		{
			pthread_exit(NULL);
		}
		++thrInfo->jobWaiting;
		pthread_mutex_unlock(&thrInfo->thrMutex);

		task->proc(pthread_self(), task->id, task->arg);
		free(task);

		//if (thrInfo->maxTskNum <= thrInfo->thrNum)
		pthread_cond_signal(&thrInfo->emptyThrLoop);

		if (0 < thrInfo->jobWaiting && 0 < thrInfo->taskNum)
			pthread_cond_signal(&thrInfo->noEmptyThrLoop);
	}

	return nullptr;
}
