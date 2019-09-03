#include "ThreadLoop.h"
#include <pthread.h>
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

	thrInfo->isClose = 0;
	thrInfo->jobWaiting = 0;
	
	thrInfo->jobThrNum = 0;

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

void ThreadLoop::addTask(void (*const & thrProc)(void *), void* const& arg)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	
	//cout << thrInfo->maxTskNum << "<=" << thrInfo->taskNum << endl;
	while (thrInfo->maxTskNum <= thrInfo->taskNum || 
		thrInfo->thrNum <= thrInfo->jobThrNum ||
		0 >= thrInfo->jobWaiting)
	{
		pthread_cond_wait(&thrInfo->emptyThrLoop, &thrInfo->condMutex);
	}
	pthread_mutex_lock(&thrInfo->thrMutex);
	//cout << "wait OK" << endl;
	++thrInfo->jobThrNum;
	--thrInfo->jobWaiting;
	__Task* task = (__Task*)malloc(sizeof(__Task));
	task->id = taskId++;
	task->proc = thrProc;
	task->arg = arg;

	thrInfo->taskArr[thrInfo->taskNum++] = task;

	pthread_mutex_unlock(&thrInfo->thrMutex);
	pthread_cond_signal(&thrInfo->noEmptyThrLoop);

	//cout << "addTask OK" << endl;
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

	__ThreadInfo* thrInfo = (__ThreadInfo*)arg;
	++thrInfo->jobWaiting;
	if (thrInfo->jobWaiting == thrInfo->thrNum)
		pthread_cond_signal(&thrInfo->emptyThrLoop);

	while (true)
	{
		//cout << "lock ok" << endl;
		while (0 >= thrInfo->taskNum && !thrInfo->isClose)
		{
			pthread_cond_wait(&thrInfo->noEmptyThrLoop, &thrInfo->condMutex);
		}
		cout << "wait OK" << endl;
		pthread_mutex_lock(&thrInfo->thrMutex);
		//cout << "deal task" << endl;
		//while (0 == thrInfo->taskNum)
		//{
		//	pthread_cond_wait(&thrInfo->emptyThrLoop, &thrInfo->thrMutex);
		//}
		int taskTmp;
		if (thrInfo->taskNum)
		{
			__DEBUGP(thrInfo->isClose);
			__DEBUGP(thrInfo->jobWaiting);
			__DEBUGP(thrInfo->jobThrNum);
			__DEBUGP(thrInfo->taskNum);
			__DEBUGP(thrInfo->taskBeg);
			//--thrInfo->jobWaiting;
			--thrInfo->taskNum;
			taskTmp = thrInfo->taskBeg;
			//__Task* task = thrInfo->taskArr[thrInfo->taskBeg];
			//cout << "task running : " << task->id << endl;
			//task->proc(task->arg);

			thrInfo->taskBeg = (thrInfo->taskBeg + 1) % thrInfo->maxTskNum;
			
		}

		if (1 == thrInfo->isClose)
		{
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&thrInfo->thrMutex);
		__Task* task = thrInfo->taskArr[taskTmp];
		
		cout << "task running : " << task->id << endl;
		task->proc(task->arg);
		--thrInfo->jobThrNum;
		++thrInfo->jobWaiting;
		cout << "task completed : " << task->id << endl;
		pthread_cond_signal(&thrInfo->emptyThrLoop);
	}

	return nullptr;
}
