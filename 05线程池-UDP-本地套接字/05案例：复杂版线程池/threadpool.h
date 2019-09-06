#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

typedef struct threadpool_t threadpool_t;

/**
 * @function threadpool_create										threadpool_create����
 * @descCreates a threadpool_t object.									����������һ��threadpool_t����
 * @param thr_num  thread num											������thr_num		�߳���
 * @param max_thr_num  max thread size									������max_thr_size	����߳���
 * @param queue_max_size   size of the queue.							������queue_max_size	������������
 * @return a newly created thread pool or NULL							���أ�һ���´������̳߳ػ���NULL
 */
threadpool_t *threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size);

/**
 * @function threadpool_add											threadpool_add����
 * @desc add a new task in the queue of a thread pool					���������̳߳ص�������������һ���µ�����
 * @param pool     Thread pool to which add the task.					������pool		Ҫ���������̳߳�
 * @param function Pointer to the function that will perform the task.	������function	ָ��ִ������ĺ���
 * @param argument Argument to be passed to the function.				������argument	���ݸ������Ĳ���
 * @return 0 if all goes well,else -1									���أ��ɹ�����0��ʧ�ܷ���-1
 */
int threadpool_add(threadpool_t *pool, void*(*function)(void *arg), void *arg);

/**
 * @function threadpool_destroy										threadpool_destroy����
 * @desc Stops and destroys a thread pool.								������ֹͣ��������һ���̳߳�
 * @param pool  Thread pool to destroy.									������pool		Ҫ���ٵ��̳߳�
 * @return 0 if destory success else -1									���أ��ɹ�����0��ʧ�ܷ���-1
 */
int threadpool_destroy(threadpool_t *pool);

/**
 *																	threadpool_all_threadnum����
 * @desc get the thread num												��������ȡ�߳���
 * @pool pool threadpool												������pool		�̳߳�
 * @return # of the thread												���أ������߳���
 */
int threadpool_all_threadnum(threadpool_t *pool);

/**
 *																	threadpool_busy_threadnum����
 * desc get the busy thread num											��������ȡæµ�߳���
 * @param pool threadpool												������pool		�̳߳�
 * return # of the busy thread											���أ�����æµ�߳���
 */
int threadpool_busy_threadnum(threadpool_t *pool);

#endif
