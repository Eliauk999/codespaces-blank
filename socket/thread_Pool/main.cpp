//Created on 2023/ 05/26
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>
#include<string.h>
#include<iostream>
#include<sys/epoll.h>
using namespace std;
/*

*/

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

    int shutdown;                   // 线程池使用状态 bool值
};

threadpool_t *threadpool_create(int min_thr_num , int max_thr_num , int queue_max_size);
void *threadpool_thread(void *threadpool);



// 初始化线程池
threadpool_t *threadpool_create(int min_thr_num , int max_thr_num , int queue_max_size){
    threadpool_t *pool =NULL;
    // do while(0) + break 用于替代goto
    do{
        if((pool = (threadpool_t *)malloc(sizeof(threadpool_t)))==NULL){
            cout<<"malloc pool err"<<endl;
            break;
        }
        pool->min_thr_num = min_thr_num;
        pool->max_thr_num = max_thr_num;
        pool->busy_thr_num = 0;
        pool->live_thr_num = min_thr_num;
        pool->wait_exit_thr_num = 0;
        pool->queue_size = 0;
        pool->queue_max_size = queue_max_size;
        pool->queue_front = 0;
        pool->queue_rear = 0;
        pool->shutdown = false;

        // 根据最大线程数的上限，给工作线程数组开辟空间并初始化
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t)*max_thr_num);
        if(pool->threads == nullptr){
            cout<<"malloc threads err"<<endl;
            break;
        }
        bzero(pool->threads,sizeof(pthread_t)*max_thr_num);

        // 给任务队列开辟空间
        pool->task_queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t)*queue_max_size);
        if(pool->task_queue == nullptr){
            cout<<"malloc task_queue err"<<endl;
            break;
        }

        // 初始化互斥锁、条件变量
        if(pthread_mutex_init(&(pool->lock),NULL)!=0 
        ||pthread_mutex_init(&(pool->thread_counter),NULL) !=0
        ||pthread_cond_init(&(pool->queue_not_full),NULL)!=0
        ||pthread_cond_init(&(pool->queue_not_empty),NULL)!=0){
            cout<<"Init locks err"<<endl;
            break;
        }

        // 启动线程
        for(int i=0;i<min_thr_num;i++){
            // pool 指向当前线程池
            pthread_create(&(pool->threads[i]),NULL,threadpool_thread,(void*)pool);
            cout<<"stread thread 0x"<<pool->threads[i]<<endl;
        }

        // 创建管理者线程
        pthread_create(&(pool->adjust_tid),NULL,adjust_thread,(void*)pool);

        return pool;
    }while(0);
    // 执行到这里说明初始化错误了
    threadpool_free(pool);
    return nullptr;
}

// 线程池中各个工作进程
void *threadpool_thread(void *threadpool){
    threadpool_t *pool = (threadpool_t*)threadpool;
    threadpool_task_t task;

    while(1){
        // 刚创建出线程，等待任务队列中出现任务，否则阻塞等待，有任务之后再唤醒
        pthread_mutex_lock(&(pool->lock));
        // 任务队列 ==0 时，调wait阻塞再条件变量上，有任务就跳过while
        while((pool->queue_size == 0)&&(!pool->shutdown)){
            cout<<"thread 0x"<<pthread_self()<<" is waiting..."<<endl;
            pthread_cond_wait(&(pool->queue_not_empty),&(pool->lock));

            // 清楚指定数目的空闲线程
            if(pool->wait_exit_thr_num > 0){
                pool->wait_exit_thr_num --;

                // 如果线程池里线程个数大于最小值，就结束当前进程
                if(pool->live_thr_num > pool->min_thr_num){
                    cout<<"thread 0x"<<pthread_self()<<" is exiting..."<<endl;
                    pool->live_thr_num--;
                    pthread_mutex_unlock(&(pool->lock));
                    pthread_exit(0);
                }
            }
        }


    }

}

// 管理者线程



int main() {
    // 创建线程池，最少3个，最多100个，队列最大100个
    threadpool_t *thp = threadpool_create(3,100,100);
    cout<<"threadpool created."<<endl;
    int num[20];
    for (int i = 0; i <20 ; i++)
    {
        num[i] = i;
        cout<<"add task:"<<i<<endl;
        // 添加任务
        threadpool_add(thp,process,(void*)&num[i]);
    }
    // 模拟工作
    sleep(5);
    // 等子进程完成任务 销毁线程池
    threadpool_destory(thp);
    return 0;
}
