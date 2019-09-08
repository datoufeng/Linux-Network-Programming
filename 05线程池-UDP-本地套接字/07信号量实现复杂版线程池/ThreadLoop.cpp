#include <string.h>
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
#endif // __DEBUG]
	
	thrInfo = (__ThreadInfo*)malloc(sizeof(__ThreadInfo));
	initTskThr(minThrNum, maxThrNum, maxTskNum);

	thrInfo->jobThrNum = 0;
	thrInfo->varyThrNum = 0;
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
	//pthread_cond_init(&thrInfo->notFull, NULL);
	//pthread_cond_init(&thrInfo->notEmpty, NULL);
	pthread_cond_init(&thrInfo->closed, NULL);
	sem_init(&thrInfo->tskSem, 0, thrInfo->tskNum);
	sem_init(&thrInfo->maxTskSem, 0, thrInfo->maxTskNum);

	thrInfo->tskArr = (__Task**)malloc(sizeof(__Task*) * thrInfo->maxTskNum);
	thrInfo->thrArr = (pthread_t*)malloc(sizeof(pthread_t) * thrInfo->maxThrNum);
	//thrInfo->thrIndexArr = (ThrSize*)malloc(sizeof(ThrSize) * thrInfo->maxThrNum);
	//printf("malloc thrArr[%p]\n", thrInfo->thrArr);


	pthread_attr_init(&thrInfo->thrAttr);
	pthread_attr_setdetachstate(&thrInfo->thrAttr, PTHREAD_CREATE_DETACHED);

	pthread_create(&thrInfo->ctrlThr, &thrInfo->thrAttr, ctrlThrProc, thrInfo);
	
	for (int i = 0; i < thrInfo->thrNum; i++)
	{
		//thrInfo->thrIndexArr[i] = i;
		pthread_create(thrInfo->thrArr + i, &thrInfo->thrAttr, thrProc, thrInfo);
	}

}


ThreadLoop::~ThreadLoop()
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	if (false == thrInfo->isClose) thrInfo->isClose = true;

	while (false == thrInfo->thrClosed || false == thrInfo->ctrlThrClosed)
	{
		pthread_cond_wait(&thrInfo->closed, &thrInfo->condMutex);
	}
	pthread_attr_destroy(&thrInfo->thrAttr);
	pthread_mutex_destroy(&thrInfo->thrMutex);
	pthread_mutex_destroy(&thrInfo->condMutex);
	pthread_cond_destroy(&thrInfo->closed);
	sem_destroy(&thrInfo->tskSem);
	sem_destroy(&thrInfo->maxTskSem);
	cout << "free" << endl;
	for (int i = 0; i < thrInfo->tskNum; i++)
	{
		__Task*& task =
			thrInfo->tskArr[(thrInfo->firstTskNum + i) % thrInfo->maxTskNum];
		if (0 != task) free(task);
	}
	//cout << "task free ok" << endl;
	free(thrInfo->tskArr);
	//cout << "tskArr free ok" << endl;
	//printf("free thrArr[%p]\n", thrInfo->thrArr);
	free(thrInfo->thrArr);
	//cout << "thrArr free ok" << endl;
	free(thrInfo);
	//cout << "thrInfo free ok" << endl;
}

int ThreadLoop::addTsk(void(*proc)(const pthread_t &thrId, const TaskId &tskId, void * const &arg), void * const & arg)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	if (thrInfo->isClose) return -1;

	__Task* task = (__Task*)malloc(sizeof(__Task));
	task->tskId = tskId++;
	task->proc = proc;
	task->arg = arg;

	sem_wait(&thrInfo->maxTskSem);

	pthread_mutex_lock(&thrInfo->thrMutex);
	thrInfo->tskArr[(thrInfo->firstTskNum + thrInfo->tskNum) % thrInfo->maxTskNum] = task;
	++thrInfo->tskNum;
	pthread_mutex_unlock(&thrInfo->thrMutex);
	
	sem_post(&thrInfo->tskSem);
}

void ThreadLoop::closeThrLoop()
{
	int thrNum = thrInfo->thrNum;
	thrInfo->isClose = true;

	for (int i = 0; i < thrNum; i++)
	{
		sem_post(&thrInfo->tskSem);
	}

	this->~ThreadLoop();
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
	int thrIndex = getThrIndex(pthread_self(), thrInfo->thrArr, thrInfo->thrNum);
	if (-1 == thrIndex)
	{
		cout << "create thread err" << endl;
		return NULL;
	}
	//cout << "thrIndex : " << thrIndex << endl;

	while (true)
	{
		__Task* task;

		if (!thrInfo->isClose)
			sem_wait(&thrInfo->tskSem);
		//cout << "sem_wait ok" << endl;

		pthread_mutex_lock(&thrInfo->thrMutex);
		++thrInfo->jobThrNum;
		if (thrInfo->isClose)
		{
			--thrInfo->thrNum;
			if (0 == thrInfo->thrNum)
			{
				cout << "all thr closed" << endl;
				thrInfo->thrClosed = true;
			}
			pthread_mutex_unlock(&thrInfo->thrMutex);
			pthread_cond_signal(&thrInfo->closed);
			pthread_exit(NULL);
		}
		if (thrInfo->tskNum)
		{
			--thrInfo->tskNum;
			task = thrInfo->tskArr[thrInfo->firstTskNum];
			thrInfo->tskArr[thrInfo->firstTskNum] = 0;
			thrInfo->firstTskNum =
				(thrInfo->maxTskNum + thrInfo->firstTskNum + 1) % 
				thrInfo->maxTskNum;
		}
		pthread_mutex_unlock(&thrInfo->thrMutex);

		sem_post(&thrInfo->maxTskSem);

		printf("[%d]\n", thrInfo->thrNum);
		task->proc(pthread_self(), task->tskId, task->arg);
		free(task);

		pthread_mutex_lock(&thrInfo->thrMutex);
		--thrInfo->jobThrNum;
		if (0 == thrInfo->varyThrNum)
			pthread_mutex_unlock(&thrInfo->thrMutex);
		else if (0 > thrInfo->varyThrNum)
		{
			++thrInfo->varyThrNum;
			--thrInfo->thrNum;
			thrInfo->thrArr[thrIndex] = 
				thrInfo->thrArr[thrInfo->thrNum];
			thrInfo->thrArr[thrInfo->thrNum] = 0;
			//cout << "thrInfo->thrNum : " << thrInfo->thrNum << endl;
			pthread_mutex_unlock(&thrInfo->thrMutex);
			pthread_exit(NULL);
		}
		else
		{
			int vary = thrInfo->varyThrNum;
			int thrNum = thrInfo->thrNum;
			thrInfo->varyThrNum = 0;
			thrInfo->thrNum += vary;
			pthread_mutex_unlock(&thrInfo->thrMutex);
			for (int i = 0; i < vary; i++)
			{
				//thrInfo->thrIndexArr[thrNum + i] = thrNum + i;
				pthread_create(thrInfo->thrArr + thrNum + i, &thrInfo->thrAttr, thrProc, thrInfo);
			}
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
		sleep(CTRL_TIMEOUT);
		if (thrInfo->isClose)
		{
			thrInfo->ctrlThrClosed = true;
			pthread_cond_signal(&thrInfo->closed);
			pthread_exit(NULL);
		}

		pthread_mutex_lock(&thrInfo->thrMutex);

		thrInfo->varyThrNum += ctrlNum(arg);
		cout << "ctrl ok " << thrInfo->varyThrNum << endl;

		pthread_mutex_unlock(&thrInfo->thrMutex);


	}

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

	//__PDEBUG(thrInfo->minThrNum);
	//__PDEBUG(thrInfo->maxThrNum);
	//__PDEBUG(thrInfo->maxTskNum);

	if (0 == minThrNum) thrInfo->minThrNum = 1;
	if (0 == maxThrNum) thrInfo->maxThrNum = 1;
}

int ThreadLoop::ctrlNum(void* const& arg)
{
	__ThreadInfo* thrInfo = (__ThreadInfo*)arg;
	
	int newThrNum = thrInfo->thrNum + thrInfo->varyThrNum;

	if (thrInfo->tskNum < thrInfo->minThrNum)
		return thrInfo->minThrNum - newThrNum;
	else if (thrInfo->tskNum > thrInfo->maxThrNum)
		return thrInfo->maxThrNum - newThrNum;
	else if (newThrNum > 3 * thrInfo->tskNum / 2)
	{
		return 
			(newThrNum / 2 > thrInfo->minThrNum ?
			newThrNum / 2 : thrInfo->minThrNum) -
			newThrNum;
	}
	else if (3 * thrInfo->thrNum / 2 < thrInfo->tskNum)
	{
		return
			(newThrNum * 2 < thrInfo->maxThrNum ?
			newThrNum * 2 : thrInfo->maxThrNum) -
			newThrNum;
	}

	return 0;
}

int ThreadLoop::getThrIndex(
	const pthread_t& thrId,
	pthread_t* const & thrArr,
	const ThrSize& thrNum)
{
	for (int i = 0; i < thrNum; i++)
	{
		if (thrId == thrArr[i]) return i;
	}

	return -1;
}
