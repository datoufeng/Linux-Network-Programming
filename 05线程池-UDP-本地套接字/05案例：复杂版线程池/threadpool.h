#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

typedef struct threadpool_t threadpool_t;

/**
 * @function threadpool_create										threadpool_create函数
 * @descCreates a threadpool_t object.									描述：创建一个threadpool_t对象
 * @param thr_num  thread num											参数：thr_num		线程数
 * @param max_thr_num  max thread size									参数：max_thr_size	最大线程数
 * @param queue_max_size   size of the queue.							参数：queue_max_size	最大任务队列数
 * @return a newly created thread pool or NULL							返回：一个新创建的线程池或者NULL
 */
threadpool_t *threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size);

/**
 * @function threadpool_add											threadpool_add函数
 * @desc add a new task in the queue of a thread pool					描述：在线程池的任务队列中添加一个新的任务
 * @param pool     Thread pool to which add the task.					参数：pool		要添加任务的线程池
 * @param function Pointer to the function that will perform the task.	参数：function	指向执行任务的函数
 * @param argument Argument to be passed to the function.				参数：argument	传递给函数的参数
 * @return 0 if all goes well,else -1									返回：成功返回0。失败返回-1
 */
int threadpool_add(threadpool_t *pool, void*(*function)(void *arg), void *arg);

/**
 * @function threadpool_destroy										threadpool_destroy函数
 * @desc Stops and destroys a thread pool.								描述：停止并且销毁一个线程池
 * @param pool  Thread pool to destroy.									参数：pool		要销毁的线程池
 * @return 0 if destory success else -1									返回：成功返回0。失败返回-1
 */
int threadpool_destroy(threadpool_t *pool);

/**
 *																	threadpool_all_threadnum函数
 * @desc get the thread num												描述：获取线程数
 * @pool pool threadpool												参数：pool		线程池
 * @return # of the thread												返回：返回线程数
 */
int threadpool_all_threadnum(threadpool_t *pool);

/**
 *																	threadpool_busy_threadnum函数
 * desc get the busy thread num											描述：获取忙碌线程数
 * @param pool threadpool												参数：pool		线程池
 * return # of the busy thread											返回：返回忙碌线程数
 */
int threadpool_busy_threadnum(threadpool_t *pool);

#endif
