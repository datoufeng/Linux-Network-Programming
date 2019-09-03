#pragma once
#include <iostream>

class ThreadLoop
{
	struct __Task
	{
		int tasknum;//模拟任务编号
		void *arg;//回调函数参数
		void(*task_func)(void *arg);//任务的回调函数
	};

	struct __ThreadInfo
	{
		int max_job_num;//最大任务个数
		int job_num;//实际任务个数
		__Task *tasks;//任务队列数组
		int job_push;//入队位置
		int job_pop;// 出队位置

		int thr_num;//线程池内线程个数
		pthread_t *threads;//线程池内线程数组
		int shutdown;//是否关闭线程池
		pthread_mutex_t pool_lock;//线程池的锁
		pthread_cond_t empty_task;//任务队列为空的条件
		pthread_cond_t not_empty_task;//任务队列不为空的条件
	};
public:
	ThreadLoop(
		const int &thrNum = defaultThrNum, 
		const int & maxThrNum = defaultMaxThrNum);
	~ThreadLoop();

	void addTask(void (*const &thrProc)(void*), void* const& arg);
	void delTask(void (*const &thrProc)(void*), void* const& arg);
	__ThreadInfo *thrInfo = NULL;
private:
	int taskId = 0;
	static const int defaultThrNum, defaultMaxThrNum;

	static void* thrProc(void*);

};

