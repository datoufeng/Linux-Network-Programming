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
	thrInfo->shutdown = 0;//�Ƿ�ݻ��̳߳أ�1����ݻ�
	thrInfo->job_push = 0;//���������ӵ�λ��
	thrInfo->job_pop = 0;//������г��ӵ�λ��
	thrInfo->job_num = 0;//��ʼ�����������Ϊ0

	thrInfo->tasks = (__Task*)malloc((sizeof(__Task)*maxTskNum));//���������������

	//��ʼ��������������
	pthread_mutex_init(&thrInfo->pool_lock, NULL);
	pthread_cond_init(&thrInfo->empty_task, NULL);
	pthread_cond_init(&thrInfo->not_empty_task, NULL);

	int i = 0;
	thrInfo->threads = (pthread_t *)malloc(sizeof(pthread_t)*thrNum);//����n���߳�id�Ŀռ�

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	for (i = 0; i < thrNum; i++)
	{
		pthread_create(&thrInfo->threads[i], &attr, thrProc, (void*)thrInfo);//��������߳�
	}
	//printf("end call %s-----\n",__FUNCTION__);
}


ThreadLoop::~ThreadLoop()
{
#ifdef __DEBUG
	cout << __FUNCTION__ << endl;
#endif // __DEBUG

	thrInfo->shutdown = 1;//��ʼ�Ա�
	pthread_cond_broadcast(&thrInfo->not_empty_task);//��ɱ 

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

	//ʵ�������������������������������ȴ�(�ȴ����񱻴���)
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

	pthread_cond_signal(&thrInfo->not_empty_task);//֪ͨ����
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
	int taskpos = 0;//����λ��
	__Task *task = (__Task *)malloc(sizeof(__Task));

	while (1)
	{
		//��ȡ������Ҫ���Լ���
		pthread_mutex_lock(&thrInfo->pool_lock);

		//���������̳߳ز���Ҫ�ݻ�
		while (thrInfo->job_num <= 0 && !thrInfo->shutdown)
		{
			//���û�������̻߳�����
			pthread_cond_wait(&thrInfo->not_empty_task, &thrInfo->pool_lock);
		}

		if (thrInfo->job_num)
		{
			//��������Ҫ����
			taskpos = (thrInfo->job_pop++) % thrInfo->max_job_num;
			//printf("task out %d...tasknum===%d tid=%lu\n",taskpos,thrPool->tasks[taskpos].tasknum,pthread_self());
			//ΪʲôҪ���������������޸ģ������߻��������
			memcpy(task, &thrInfo->tasks[taskpos], sizeof(__Task));
			task->arg = task;
			thrInfo->job_num--;
			//task = &thrPool->tasks[taskpos];
			pthread_cond_signal(&thrInfo->empty_task);//֪ͨ������
		}

		if (thrInfo->shutdown)
		{
			//����Ҫ�ݻ��̳߳أ���ʱ�߳��˳�����
			//pthread_detach(pthread_self());//����ǰ�ּ�
			pthread_mutex_unlock(&thrInfo->pool_lock);
			free(task);
			pthread_exit(NULL);
		}

		//�ͷ���
		pthread_mutex_unlock(&thrInfo->pool_lock);
		task->task_func(task->arg);//ִ�лص�����
	}

	//printf("end call %s-----\n",__FUNCTION__);

	return nullptr;
}
