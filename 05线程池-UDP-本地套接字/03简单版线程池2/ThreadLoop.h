#pragma once
#include <iostream>

typedef unsigned long TaskId;
class ThreadLoop
{

	struct __Task
	{
		TaskId id;
		void* arg;
		void (*proc)(pthread_t, TaskId, void*);
	};

	struct __ThreadInfo
	{
		pthread_t *thrArr;
		__Task **taskArr;

		int thrNum;
		int maxTskNum;

		int isClose;
		int jobWaiting;

		//int jobThrNum;

		int taskNum;
		int taskBeg;

		pthread_mutex_t thrMutex;
		pthread_mutex_t condMutex;
		pthread_cond_t emptyThrLoop;
		pthread_cond_t noEmptyThrLoop;
	};
public:
	ThreadLoop(
		const int &thrNum = defaultThrNum, 
		const int & maxThrNum = defaultMaxThrNum);
	~ThreadLoop();

	void addTask(void (*const &thrProc)(pthread_t, TaskId, void*), void* const& arg);
	void delTask(void (*const &thrProc)(void*), void* const& arg);
	__ThreadInfo *thrInfo = NULL;
private:
	TaskId taskId = 0;
	static const int defaultThrNum, defaultMaxThrNum;

	static void* thrProc(void*);

};

