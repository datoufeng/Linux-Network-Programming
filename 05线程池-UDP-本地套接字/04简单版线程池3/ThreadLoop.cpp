#include "ThreadLoop.h"
#include <string.h>
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
	

	printf("begin call %s-----\n", __FUNCTION__);
	thrInfo = (__ThreadInfo*)malloc(sizeof(__ThreadInfo));

	thrInfo->thr_num = thrNum;
	thrInfo->max_job_num = maxTskNum;
	thrInfo->shutdown = 0;//是否摧毁线程池，1代表摧毁
	thrInfo->job_push = 0;//任务队列添加的位置
	thrInfo->job_pop = 0;//任务队列出队的位置
	thrInfo->job_num = 0;//初始化的任务个数为0

	thrInfo->tasks = (__Task*)malloc((sizeof(__Task)*maxTskNum));//申请最大的任务队列

	//初始化锁和条件变量
	pthread_mutex_init(&thrInfo->pool_lock, NULL);
	pthread_cond_init(&thrInfo->empty_task, NULL);
	pthread_cond_init(&thrInfo->not_empty_task, NULL);

	int i = 0;
	thrInfo->threads = (pthread_t *)malloc(sizeof(pthread_t)*thrNum);//申请n个线程id的空间

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	for (i = 0; i < thrNum; i++)
	{
		pthread_create(&thrInfo->threads[i], &attr, thrProc, (void*)thrInfo);//创建多个线程
	}
	//printf("end call %s-----\n",__FUNCTION__);
}


ThreadLoop::~ThreadLoop()
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	thrInfo->shutdown = 1;//开始自爆
	pthread_cond_broadcast(&thrInfo->not_empty_task);//诱杀 

	int i = 0;
	for (i = 0; i < thrInfo->thr_num; i++)
	{
		pthread_join(thrInfo->threads[i], NULL);
	}

	pthread_cond_destroy(&thrInfo->not_empty_task);
	pthread_cond_destroy(&thrInfo->empty_task);
	pthread_mutex_destroy(&thrInfo->pool_lock);

	free(thrInfo->tasks);
	free(thrInfo->threads);
	free(thrInfo);
}

void ThreadLoop::addTask(void (*const & thrProc)(void *), void* const& arg)
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	//printf("begin call %s-----\n",__FUNCTION__);
	pthread_mutex_lock(&thrInfo->pool_lock);

	//实际任务总数大于最大任务个数则阻塞等待(等待任务被处理)
	while (thrInfo->max_job_num <= thrInfo->job_num)
	{
		pthread_cond_wait(&thrInfo->empty_task, &thrInfo->pool_lock);
	}

	int taskpos = (thrInfo->job_push++) % thrInfo->max_job_num;
	//printf("add task %d  tasknum===%d\n",taskpos,beginnum);
	thrInfo->tasks[taskpos].tasknum = taskId++;
	thrInfo->tasks[taskpos].arg = (void*)&thrInfo->tasks[taskpos];
	thrInfo->tasks[taskpos].task_func = thrProc;
	thrInfo->job_num++;

	pthread_mutex_unlock(&thrInfo->pool_lock);

	pthread_cond_signal(&thrInfo->not_empty_task);//通知包身工
	//printf("end call %s-----\n",__FUNCTION__);
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

	//printf("begin call %s-----\n",__FUNCTION__);
	__ThreadInfo *thrInfo = (__ThreadInfo*)arg;
	int taskpos = 0;//任务位置
	__Task *task = (__Task *)malloc(sizeof(__Task));

	while (1)
	{
		//获取任务，先要尝试加锁
		pthread_mutex_lock(&thrInfo->pool_lock);

		//无任务并且线程池不是要摧毁
		while (thrInfo->job_num <= 0 && !thrInfo->shutdown)
		{
			//如果没有任务，线程会阻塞
			pthread_cond_wait(&thrInfo->not_empty_task, &thrInfo->pool_lock);
		}

		if (thrInfo->job_num)
		{
			//有任务需要处理
			taskpos = (thrInfo->job_pop++) % thrInfo->max_job_num;
			//printf("task out %d...tasknum===%d tid=%lu\n",taskpos,thrPool->tasks[taskpos].tasknum,pthread_self());
			//为什么要拷贝？避免任务被修改，生产者会添加任务
			memcpy(task, &thrInfo->tasks[taskpos], sizeof(__Task));
			task->arg = task;
			thrInfo->job_num--;
			//task = &thrPool->tasks[taskpos];
			pthread_cond_signal(&thrInfo->empty_task);//通知生产者
		}

		if (thrInfo->shutdown)
		{
			//代表要摧毁线程池，此时线程退出即可
			//pthread_detach(pthread_self());//临死前分家
			pthread_mutex_unlock(&thrInfo->pool_lock);
			free(task);
			pthread_exit(NULL);
		}

		//释放锁
		pthread_mutex_unlock(&thrInfo->pool_lock);
		task->task_func(task->arg);//执行回调函数
	}

	//printf("end call %s-----\n",__FUNCTION__);

	return nullptr;
}
