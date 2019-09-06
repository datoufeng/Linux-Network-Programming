#include <string.h>
#include <pthread.h>

#include <unistd.h>

#include "ThreadLoop.h"

//#define __DEBUG
#define __PDEBUG(X) printf(#X" : %u\n", X)


using namespace std;

ThreadLoop::ThreadLoop(
	const ThrSize& minThrNum,
	const ThrSize& maxThrNum,
	const ThrSize& maxTskNum)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	thrInfo = (__ThreadInfo*)malloc(sizeof(__ThreadInfo));
	initTskThr(minThrNum, maxThrNum, maxTskNum);

	thrInfo->jobThrNum = 0;
	thrInfo->thrNum = (thrInfo->minThrNum + thrInfo->maxThrNum) / 2;
	
	thrInfo->firstTskNum = 0;
	thrInfo->tskNum = 0;

	thrInfo->isClose = false;
	thrInfo->thrClosed = false;
	thrInfo->ctrlThrClosed = false;

	pthread_mutex_init(&thrInfo->thrMutex, NULL);
	pthread_mutex_init(&thrInfo->condMutex, NULL);
	//pthread_mutex_init(&thrInfo->notFullCondMutex, NULL);
	//pthread_mutex_init(&thrInfo->notEmptyCondMutex, NULL);
	pthread_cond_init(&thrInfo->notFull, NULL);
	pthread_cond_init(&thrInfo->notEmpty, NULL);

	thrInfo->tskArr = (__Task**)malloc(sizeof(__Task*));
	thrInfo->thrArr = (pthread_t*)malloc(sizeof(pthread_t));

	pthread_attr_t thrAttr;
	pthread_attr_init(&thrAttr);
	pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);

	pthread_create(&thrInfo->ctrlThr, &thrAttr, ctrlThrProc, thrInfo);
	for (int i = 0; i < thrInfo->thrNum; i++)
	{
		pthread_create(thrInfo->thrArr + i, &thrAttr, thrProc, thrInfo);
	}

	pthread_attr_destroy(&thrAttr);
}


ThreadLoop::~ThreadLoop()
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG


	while (false == thrInfo->thrClosed || false == thrInfo->ctrlThrClosed)
	{
		pthread_cond_wait(&thrInfo->closed, &thrInfo->condMutex);
	}
	for (int i = 0; i < thrInfo->tskNum; i++)
	{
		free(thrInfo->tskArr[(thrInfo->maxTskNum + thrInfo->firstTskNum + i) % thrInfo->maxTskNum]);
	}
	free(thrInfo->tskArr);
	free(thrInfo->thrArr);
	free(thrInfo);
}

int ThreadLoop::addTsk(void(*proc)(const pthread_t &thrId, const TaskId &tskId, void * const &arg), void * const & arg)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG


	__Task* task = (__Task*)malloc(sizeof(__Task));
	task->tskId = tskId++;
	task->proc = proc;
	task->arg = arg;

	pthread_mutex_lock(&thrInfo->thrMutex);

	while (thrInfo->maxTskNum <= thrInfo->tskNum)
	{
		cout << "addTask wait" << endl;
		pthread_mutex_unlock(&thrInfo->thrMutex);
		pthread_cond_wait(&thrInfo->notFull, &thrInfo->condMutex);
		pthread_mutex_lock(&thrInfo->thrMutex);
	}

	thrInfo->tskArr[(thrInfo->maxTskNum + thrInfo->firstTskNum + thrInfo->tskNum) % thrInfo->maxTskNum] = task;
	++thrInfo->tskNum;
	int signalThrNum = thrInfo->thrNum - thrInfo->jobThrNum;
	//cout << "thrNum : " << thrInfo->thrNum << endl;
	//cout << "jobThrNum : " << thrInfo->jobThrNum << endl;
	//cout << "signalThrNum : " << signalThrNum << endl;
 	int tskNum = thrInfo->tskNum;
	//cout << "tskNum : " << tskNum << endl;
	pthread_mutex_unlock(&thrInfo->thrMutex);
	pthread_cond_broadcast(&thrInfo->notEmpty);

	//for (int i = 0; i < (signalThrNum < tskNum ? signalThrNum : tskNum); i++)
	//{
	//	//cout << "addTask signal" << endl;
	//	pthread_cond_signal(&thrInfo->notEmpty);
	//	//cout << "addTask signal ok" << endl;
	//}

}

void * ThreadLoop::thrProc(void * arg)
{
#ifdef __DEBUG
	char buf[64] = { 0 };
	sprintf(buf, "%s[%u]\n", __FUNCTION__, pthread_self());
	write(STDOUT_FILENO, buf, strlen(buf));
	//cout << __FUNCTION__ << "[" << pthread_self() << "]" << endl;
#endif // __DEBUG


	__ThreadInfo* thrInfo = (__ThreadInfo*)arg;
	while (true)
	{
		__Task* task;

		pthread_mutex_lock(&thrInfo->thrMutex);
		//cout << "thrProc lock1 ok" << endl;
		while (0 == thrInfo->tskNum && !thrInfo->isClose)
		{
			cout << "thrProc wait " << endl;
			pthread_mutex_unlock(&thrInfo->thrMutex);
			cout << "unlock ok" << endl;
			pthread_cond_wait(&thrInfo->notEmpty, &thrInfo->condMutex);
			cout << "wait ok" << endl;
			pthread_mutex_lock(&thrInfo->thrMutex);
			cout << "lock ok" << endl;
		}
		++thrInfo->jobThrNum;
	
		if (thrInfo->tskNum)
		{
			--thrInfo->tskNum;
			task = thrInfo->tskArr[thrInfo->firstTskNum];
			//__PDEBUG(thrInfo->firstTskNum);
			thrInfo->tskArr[thrInfo->firstTskNum] = 0;
			thrInfo->firstTskNum =
				(thrInfo->maxTskNum + thrInfo->firstTskNum + 1) % 
				thrInfo->maxTskNum;
			//__PDEBUG(thrInfo->firstTskNum);
		}

		if (thrInfo->isClose)
		{
			--thrInfo->thrNum;
			if (0 == thrInfo->thrNum)
			{
				thrInfo->thrClosed = true;
				pthread_cond_signal(&thrInfo->closed);
			}
			pthread_mutex_unlock(&thrInfo->thrMutex);
			pthread_exit(NULL);
		}

		pthread_mutex_unlock(&thrInfo->thrMutex);
		//cout << "thrProc unlock1 ok" << endl;

		task->proc(pthread_self(), task->tskId, task->arg);
	
		pthread_mutex_lock(&thrInfo->thrMutex);
		//cout << "thrProc lock2 ok" << endl;
		--thrInfo->jobThrNum;
		pthread_mutex_unlock(&thrInfo->thrMutex);
		//cout << "thrProc unlock2 ok" << endl;

		pthread_cond_signal(&thrInfo->notFull);
		if (0 != thrInfo->tskNum &&
			thrInfo->thrNum != thrInfo->jobThrNum)
		{
			cout << "broadcast notEmpty" << endl;
			pthread_cond_broadcast(&thrInfo->notEmpty);
		}

	}

	return nullptr;
}

void * ThreadLoop::ctrlThrProc(void * arg)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG


	__ThreadInfo* thrInfo = (__ThreadInfo*)arg;


	while (true)
	{

	}
	//pthread_mutex_lock(&thrInfo->thrMutex);


	//pthread_mutex_unlock(&thrInfo->thrMutex);

	return nullptr;
}

//void * ThreadLoop::addTskProc(void * arg)
//{
//	return nullptr;
//}

void ThreadLoop::initTskThr(const ThrSize & minThrNum, const ThrSize & maxThrNum, const ThrSize & maxTskNum)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG


	if (minThrNum > maxThrNum)
	{
		thrInfo->minThrNum = DEFAULT_MIN_THR_NUM;
		thrInfo->maxThrNum = DEFAULT_MAX_THR_NUM;
		thrInfo->maxTskNum = DEFAULT_MAX_TSK_NUM;
	}

	thrInfo->minThrNum = minThrNum;
	thrInfo->maxThrNum = maxThrNum;
	thrInfo->maxTskNum = maxTskNum;

	if (0 == minThrNum) thrInfo->minThrNum = 1;
	if (0 == maxThrNum) thrInfo->maxThrNum = 1;
}
