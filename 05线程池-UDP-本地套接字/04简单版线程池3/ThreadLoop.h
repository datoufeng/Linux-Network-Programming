#pragma once
#include <iostream>

class ThreadLoop
{
	struct __Task
	{
		int tasknum;//ģ��������
		void *arg;//�ص���������
		void(*task_func)(void *arg);//����Ļص�����
	};

	struct __ThreadInfo
	{
		int max_job_num;//����������
		int job_num;//ʵ���������
		__Task *tasks;//�����������
		int job_push;//���λ��
		int job_pop;// ����λ��

		int thr_num;//�̳߳����̸߳���
		pthread_t *threads;//�̳߳����߳�����
		int shutdown;//�Ƿ�ر��̳߳�
		pthread_mutex_t pool_lock;//�̳߳ص���
		pthread_cond_t empty_task;//�������Ϊ�յ�����
		pthread_cond_t not_empty_task;//������в�Ϊ�յ�����
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

