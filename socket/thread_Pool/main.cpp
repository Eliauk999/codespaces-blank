//Created on 2023/ 05/26
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
/*

*/

// 系统参数

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

    bool shutdown;                   // 线程池使用状态 bool值
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


// 创建线程池
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

        // 启动 min_thr_num 个工作线程
        for(int i=0;i<min_thr_num;i++){
            // pool 指向当前线程池
            pthread_create(&(pool->threads[i]),NULL,threadpool_thread,(void*)pool);
            printf("create thread 0x%x\n",(unsigned int)pool->threads[i]);
        }

        // 创建管理者线程
        pthread_create(&(pool->adjust_tid),NULL,adjust_thread,(void*)pool);

        return pool;
    }while(0);
    // 执行到这里说明初始化错误了
    threadpool_free(pool);
    return nullptr;
}

// 线程池中的工作线程
void *threadpool_thread(void *threadpool){
    threadpool_t *pool = (threadpool_t*)threadpool;
    threadpool_task_t task;

        // 刚创建出线程，等待任务队列中出现任务，否则阻塞等待，有任务之后再唤醒
    while(1){
        // 给整个结构体加锁
        pthread_mutex_lock(&(pool->lock));
        // 任务队列 ==0 时，调wait阻塞再条件变量上，有任务就跳过while
        while((pool->queue_size == 0)&&(!pool->shutdown)){
            // cout<<"thread 0x"<<pthread_self()<<" is waiting..."<<endl;
            printf("thread 0x%x is waiting . \n", (unsigned int)pthread_self());

            // 如果任务队列为空，将在这里阻塞。
            pthread_cond_wait(&(pool->queue_not_empty),&(pool->lock));

            // 清理多余的空闲线程
            if(pool->wait_exit_thr_num > 0){
                pool->wait_exit_thr_num --;

                // 如果线程池里线程个数大于最小值，就结束当前进程
                if (pool->live_thr_num > pool->min_thr_num)
                {
                    // cout<<"thread 0x"<<pthread_self()<<" is exiting..."<<endl;v
                    printf("thread 0x%x is exiting by threadpool_thread\n", (unsigned int)pthread_self());

                    pool->live_thr_num--;
                    pthread_mutex_unlock(&(pool->lock));
                    pthread_exit(0);
                }
            }
        }

        // 如果线程池终止，关闭自己
        if(pool->shutdown){
            pthread_mutex_unlock(&(pool->lock));
            printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }

        // 从任务队列中获取任务，
        task.function = pool->task_queue[pool->queue_front].function;
        task.arg = pool->task_queue[pool->queue_front].arg;

        // 执行出队操作 模拟环形队列
        pool->queue_front = (pool->queue_front +1) % pool->queue_max_size;
        pool->queue_size--;

        // 通知管理者可以添加新任务
        pthread_cond_broadcast(&(pool->queue_not_full));
        // 取出任务后立刻将线程池锁释放
        pthread_mutex_unlock(&(pool->lock));

        // 更新工作线程计数
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num++;
        pthread_mutex_unlock(&(pool->thread_counter));

        // 执行任务  --sleep(1) 结束后返回这里
        // task.function(task.arg);
        (*(task.function))(task.arg);

        // 任务结束处理
        printf("thread 0x%x finish working . \n", (unsigned int)pthread_self());
        // 更新工作线程计数
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num--;
        pthread_mutex_unlock(&(pool->thread_counter));

    }
    pthread_exit(0);
}

// 管理者线程
void *adjust_thread(void *threadpool){
    threadpool_t *pool = (threadpool_t*)threadpool;
    while(!pool->shutdown){
        // 定时管理线程池
        sleep(DEFAULT_TIME);
        pthread_mutex_lock(&(pool->lock));
        // 任务队列长度
        int queue_size = pool->queue_size;
        // 存活线程数量
        int live_thr_num =  pool->live_thr_num;
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->thread_counter));
        // 工作线程数量
        int busy_thr_num =  pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->thread_counter));

        // 创建新线程 算法：任务数量 > 最小线程池个数，且存活的线程数 < 最大线程数
        if(queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num){
            cout<<"try to create new threads"<<endl;
            pthread_mutex_lock(&(pool->lock));
            // 一次添加 DEFAULT_THREAD个线程
            int add =0;
            for(int i=0;i<pool->max_thr_num 
                && add<DEFAULT_THREAD_VARY
                && pool->live_thr_num < pool->max_thr_num ;i++ ){
                    cout<<"create num "<<i<<endl;
                if(pool->threads[i] == 0|| !is_thread_alive(pool->threads[i])){
                    pthread_create(&(pool->threads[i]),NULL,threadpool_thread,(void*)pool);
                    add++;
                    pool->live_thr_num++;
                }
            }
            pthread_mutex_unlock(&(pool->lock));
        }

        // 销毁多余的空闲线程 算法： 忙线程 *2 < 存活的线程  且  存活线程 > 最小线程数
        if ((busy_thr_num *2)<live_thr_num && live_thr_num > pool->min_thr_num){
            // 一次清理 DEFAULT_THREAD_VARY 个线程，随机
            pthread_mutex_lock(&(pool->lock));
            // 等待销毁的线程
            pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
            pthread_mutex_unlock(&(pool->lock));
            // 通吃处于空闲状态的线程，他们会自动终止
            for(int i=0;i<DEFAULT_THREAD_VARY;i++){
                pthread_cond_signal(&(pool->queue_not_empty));
            }
        }
    }
    return NULL;
}

// 处理线程业务的回调函数
void *process(void *arg)
{
    printf("thread 0x%x working on task %d\n", (unsigned int)pthread_self(), (int)arg);
    sleep(1);
    printf("thread 0x%x finished \n", (unsigned int)pthread_self());
    // 返回到调用者的函数  --threadpool_thread
    return NULL;
}

// 向线程池中添加一个任务
int threadpool_add(threadpool_t *pool,void*(*fun)(void* arg),void* arg){
    pthread_mutex_lock(&(pool->lock));
    // 任务队列满，调用wait阻塞
    while((pool->queue_size == pool->queue_max_size)&& (!pool->shutdown)){
        pthread_cond_wait(&(pool->queue_not_full),&(pool->lock));
    }
    if(pool->shutdown){
        pthread_cond_broadcast(&(pool->queue_not_empty));
        pthread_mutex_unlock(&pool->lock);
        return 0;
    }
    // 清空工作线程函数的参数
    if(pool->task_queue[pool->queue_rear].arg !=NULL){
        pool->task_queue[pool->queue_rear].arg =NULL;
    }
    // 添加任务到队列里
    pool->task_queue[pool->queue_rear].function = fun;
    pool->task_queue[pool->queue_rear].arg = arg;
    // 队尾指针移动，模拟环形队列
    pool->queue_rear = (pool->queue_rear +1 ) % pool->queue_max_size;
    // 添加一个任务，实际任务数+1
    pool->queue_size++;
    // 队列不可能还为空 唤醒阻塞的线程
    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));
    return 0;
}

// 线程是否存活
int is_thread_alive(pthread_t tid){
    // 向这个进程发送一个信号  0是保留信号，用来判断线程是否还存在
    int ret = pthread_kill(tid,0);
    if(ret ==ESRCH){
        return 0;//线程不存在
    }
    else{
        return 1;
    }
}

// 清空线程池
int threadpool_free(threadpool_t *pool){
    if(pool ==NULL){
        return -1;
    }
    if(pool->task_queue != NULL){
        delete pool->task_queue;
        pool->task_queue = NULL;
    }
    if(pool->threads != NULL){
        delete pool->threads ;
        pool->threads =NULL;
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_lock(&(pool->thread_counter));
        pthread_mutex_destroy(&(pool->thread_counter));
        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }
    delete pool;
    pool = NULL;
    return 0;
}

// 保留
int threadpool_all_threadnum(threadpool_t *pool){
    // 总线程数量
    int all_threadnum = -1;
    pthread_mutex_lock(&(pool->lock));
    // 存活线程数量
    all_threadnum = pool->live_thr_num;
    pthread_mutex_unlock(&(pool->lock));
    return all_threadnum;
}

// 销毁线程池，失败返回-1 成功返回0
int threadpool_destory(threadpool_t *pool){
    if(pool==NULL){
        return -1;
    }
    pool->shutdown = true;
    // 销毁管理者线程
    pthread_join(pool->adjust_tid,NULL);
    // 通知所有空闲线程 注意是空闲
    for(int i =0; i<pool->live_thr_num;i++){
        // 原理是让空闲线程有任务，结果被唤醒后发现没有任务，只能自杀
        pthread_cond_broadcast(&(pool->queue_not_empty));
    }
    for(int i =0; i<pool->live_thr_num;i++){
        pthread_join(pool->threads[i],NULL);
    }
    threadpool_free(pool);
    return 0;
}


int main() {
    // 创建线程池，最少3个，最多100个，队列最大100个
    threadpool_t *thp = threadpool_create(10,100,100);
    cout<<"threadpool created."<<endl;
    int num[40];
    for (int i = 0; i <40 ; i++)
    {
         // 模拟工作
        num[i] = i;
        cout<<"add task:"<<i<<endl;
        // 添加任务
        threadpool_add(thp,process,(void*)num[i]);
    }
   
    // sleep(5);
    while(1){
        sleep(3);
    }
    // 等子进程完成任务 销毁线程池
    threadpool_destory(thp);
    return 0;
}
