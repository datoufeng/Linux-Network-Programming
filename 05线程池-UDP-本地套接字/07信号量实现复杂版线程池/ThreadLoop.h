#pragma once
#include <pthread.h>
#include <semaphore.h>
#include <iostream>

typedef __uint32_t ThrSize;
typedef ThrSize TaskId;

constexpr ThrSize DEFAULT_MIN_THR_NUM = 5;
constexpr ThrSize DEFAULT_MAX_THR_NUM = 10;
constexpr ThrSize DEFAULT_MAX_TSK_NUM = 20;
constexpr ThrSize CTRL_TIMEOUT = 5;

class ThreadLoop
{
	struct __Task
	{
		//pthread_t thrId;
		TaskId tskId;
		void* arg;
		void(*proc)(
			const pthread_t& thrId, 
			const TaskId& tskId, 
			void* const& arg);
	};
	struct __ThreadInfo
	{
		ThrSize minThrNum;
		ThrSize maxThrNum;
		ThrSize maxTskNum;

		ThrSize jobThrNum;
		ThrSize varyThrNum;
		ThrSize thrNum;
		
		ThrSize firstTskNum;
		ThrSize tskNum;

		pthread_t ctrlThr;
		pthread_t* thrArr;
		pthread_attr_t thrAttr;
		__Task** tskArr;

		bool isClose;
		bool thrClosed;
		bool ctrlThrClosed;

		pthread_mutex_t thrMutex;
		pthread_mutex_t condMutex;
		//pthread_mutex_t notFullCondMutex;
		//pthread_mutex_t notEmptyCondMutex;
		//pthread_cond_t notFull;
		//pthread_cond_t notEmpty;
		pthread_cond_t closed;
		sem_t tskSem;
		sem_t maxTskSem;
	};
public:
	ThreadLoop(
		const ThrSize& minThrNum = DEFAULT_MIN_THR_NUM,
		const ThrSize& maxThrNum = DEFAULT_MAX_THR_NUM,
		const ThrSize& maxTskNum = DEFAULT_MAX_TSK_NUM);
	~ThreadLoop();

	int addTsk(
		void (*proc)(
			const pthread_t& thrId,
			const TaskId& tskId,
			void* const& arg),
		void* const & arg);
	void closeThrLoop();
private:
	TaskId tskId = 0;
	__ThreadInfo* thrInfo;

	static void* thrProc(void* arg);
	static void* ctrlThrProc(void* arg);

	//static void* addTskProc(void* arg);

	void initTskThr(
		const ThrSize& minThrNum,
		const ThrSize& maxThrNum,
		const ThrSize& maxTskNum);
	static int ctrlNum(void* const& arg);
};

