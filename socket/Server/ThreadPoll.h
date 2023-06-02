#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>
#include<string.h>
#include<iostream>
#include<sys/epoll.h>
using namespace std;
// 默认10秒清理一次线程池
#define DEFAULT_TIME 1
// 最小任务数量
#define MIN_WAIT_TASK_NUM 10
// 默认一次增删的线程数
#define DEFAULT_THREAD_VARY 10


typedef struct{
    void*(*function)(void*);        // 回调函数的函数指针
    void *arg;                      // 回调函数的参数
}threadpool_task_t;


// 描述线程池的相关信息
class threadpool_t{
public:
    pthread_mutex_t lock;           // 用于锁住本结构体
    pthread_mutex_t thread_counter; // 记录busy线程个数的锁

    pthread_cond_t queue_not_full;  // 当任务队列满时，添加任务的线程阻塞，等待此条件变量
    pthread_cond_t queue_not_empty; // 任务队列不为空时，通知等待任务的线程

    pthread_t *threads;             // 存放线程池中所有线程tid的数组
    pthread_t adjust_tid;           // 存管理线程的tid
    threadpool_task_t *task_queue;  // 任务队列（数组首地址）

    int min_thr_num;                // 线程池最小线程数
    int max_thr_num;                // 线程池最大线程数
    int live_thr_num;               // 当前存活线程个数
    int busy_thr_num;               // 当前工作线程个数
    int wait_exit_thr_num;          // 要销毁的线程个数

    int queue_front;                // task_queue队头下标
    int queue_rear;                 // task_queue队尾下标
    int queue_size;                 // task_queue实际任务数量
    int queue_max_size;             // task_queue可容纳的任务数量上限

    bool shutdown;                   // 线程池使用状态 bool值'

    bool ThreadPoolInit();
    int addTask(void(*fun)(void* arg),void* arg);
    int is_thread_alive(pthread_t tid);
    static void *createThread(void *);
    static void *createManagerThread(void *);
};



// 创建线程池
threadpool_t *threadpool_create(int min_thr_num , int max_thr_num , int queue_max_size);
// 回收线程池空间
int threadpool_free(threadpool_t *pool);
// 销毁线程池
int threadpool_destory(threadpool_t *pool);

// 工作线程的执行函数
void *threadpool_thread(void *threadpool);
// 管理者线程的执行函数
void *adjust_thread(void *threadpool);
// 处理线程业务的回调函数
void *process(void *arg);
// 向线程池中添加任务
int threadpool_add(threadpool_t *pool,void*(*fun)(void* arg),void* arg);
// 判断线程是否存活
int is_thread_alive(pthread_t tid);