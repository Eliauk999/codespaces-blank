#include "ThreadPoll.h"

#define MIN_THREAD_NUM 4
#define MAX_THREAD_NUM 100
#define MAX_TASK_NUM 100

#include<netinet/in.h>
#include<arpa/inet.h>

bool threadpool_t::ThreadPoolInit()
{
    do
    {
        this->min_thr_num = MIN_THREAD_NUM;
        this->max_thr_num = MAX_THREAD_NUM;
        this->busy_thr_num = 0;
        this->live_thr_num = MIN_THREAD_NUM;
        this->wait_exit_thr_num = 0;
        this->queue_size = 0;
        this->queue_max_size = queue_max_size;
        this->queue_front = 0;
        this->queue_rear = 0;
        this->shutdown = false;

        this->threads = new pthread_t[max_thr_num];
        this->task_queue = new threadpool_task_t[queue_max_size];
        bzero(this->threads, sizeof(pthread_t) * max_thr_num);
        bzero(this->threads, sizeof(threadpool_task_t) * queue_max_size);

        // 初始化互斥锁、条件变量
        if (pthread_mutex_init(&(this->lock), NULL) != 0 || pthread_mutex_init(&(this->thread_counter), NULL) != 0 || pthread_cond_init(&(this->queue_not_full), NULL) != 0 || pthread_cond_init(&(this->queue_not_empty), NULL) != 0)
        {
            cout << "Init locks err" << endl;
            break;
        }

        // 启动 min_thr_num 个工作线程
        for (int i = 0; i < min_thr_num; i++)
        {
            // this 指向当前线程池
            pthread_create(&(this->threads[i]), NULL, createThread, this);
            printf("create thread 0x%x\n", (unsigned int)this->threads[i]);
        }

        // 创建管理者线程
        pthread_create(&(this->adjust_tid), NULL, createManagerThread, (void *)this);
        return true;
    } while (0);
    // 执行到这里说明初始化错误了
    return false;
}

struct mybuffer{
    int len;
    char *buf;
    int fd;
};

void * senddata(struct mybuffer* buffer){
    send(buffer->fd,buffer->buf,buffer->len,0);
    return nullptr;
}

void *recvdata(struct mybuffer* buffer){
    recv(buffer->fd,buffer->buf,buffer->len,0);
    return nullptr;
}

int threadpool_t::addTask(void (*fun)(void *arg), void *arg)
{
    pthread_mutex_lock(&(this->lock));
    // 任务队列满，调用wait阻塞
    while ((this->queue_size == this->queue_max_size) && (!this->shutdown))
    {
        pthread_cond_wait(&(this->queue_not_full), &(this->lock));
    }
    if (this->shutdown)
    {
        pthread_cond_broadcast(&(this->queue_not_empty));
        pthread_mutex_unlock(&this->lock);
        return 0;
    }
    // 清空工作线程函数的参数
    if (this->task_queue[this->queue_rear].arg != NULL)
    {
        this->task_queue[this->queue_rear].arg = NULL;
    }
    // 添加任务到队列里
    this->task_queue[this->queue_rear].function = fun;
    this->task_queue[this->queue_rear].arg = arg;
    // 队尾指针移动，模拟环形队列
    this->queue_rear = (this->queue_rear + 1) % this->queue_max_size;
    // 添加一个任务，实际任务数+1
    this->queue_size++;
    // 队列不可能还为空 唤醒阻塞的线程
    pthread_cond_signal(&(this->queue_not_empty));
    pthread_mutex_unlock(&(this->lock));
    return 0;
}

int threadpool_t::is_thread_alive(pthread_t tid)
{
    int ret = pthread_kill(tid, 0);
    if (ret == ESRCH)
    {
        return 0; // 线程不存在
    }
    else
    {
        return 1;
    }
}

void *threadpool_t::createThread(void *pool)
{
    threadpool_t *p = (threadpool_t *)pool;
    threadpool_task_t task;

    // 刚创建出线程，等待任务队列中出现任务，否则阻塞等待，有任务之后再唤醒
    while (1)
    {
        // 给整个结构体加锁
        pthread_mutex_lock(&(p->lock));
        // 任务队列 ==0 时，调wait阻塞再条件变量上，有任务就跳过while
        while ((p->queue_size == 0) && (!p->shutdown))
        {
            // cout<<"thread 0x"<<pthread_self()<<" is waiting..."<<endl;
            printf("thread 0x%x is waiting . \n", (unsigned int)pthread_self());

            // 如果任务队列为空，将在这里阻塞。
            pthread_cond_wait(&(p->queue_not_empty), &(p->lock));

            // 清理多余的空闲线程
            if (p->wait_exit_thr_num > 0)
            {
                p->wait_exit_thr_num--;

                // 如果线程池里线程个数大于最小值，就结束当前进程
                if (p->live_thr_num > p->min_thr_num)
                {
                    // cout<<"thread 0x"<<pthread_self()<<" is exiting..."<<endl;v
                    printf("thread 0x%x is exiting by threadpool_thread\n", (unsigned int)pthread_self());
                    p->live_thr_num--;
                    pthread_mutex_unlock(&(p->lock));
                    pthread_exit(0);
                }
            }
        }

        // 如果线程池终止，关闭自己
        if (p->shutdown)
        {
            pthread_mutex_unlock(&(p->lock));
            printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }

        // 从任务队列中获取任务，
        task.function = p->task_queue[p->queue_front].function;
        task.arg = p->task_queue[p->queue_front].arg;

        // 执行出队操作 模拟环形队列
        p->queue_front = (p->queue_front + 1) % p->queue_max_size;
        p->queue_size--;

        // 通知管理者可以添加新任务
        pthread_cond_broadcast(&(p->queue_not_full));
        // 取出任务后立刻将线程池锁释放
        pthread_mutex_unlock(&(p->lock));

        // 更新工作线程计数
        pthread_mutex_lock(&(p->thread_counter));
        p->busy_thr_num++;
        pthread_mutex_unlock(&(p->thread_counter));

        // 执行任务  --sleep(1) 结束后返回这里
        // task.function(task.arg);
        (*(task.function))(task.arg);

        // 任务结束处理
        printf("thread 0x%x finish working . \n", (unsigned int)pthread_self());
        // 更新工作线程计数
        pthread_mutex_lock(&(p->thread_counter));
        p->busy_thr_num--;
        pthread_mutex_unlock(&(p->thread_counter));
    }
    pthread_exit(0);
}

void *threadpool_t::createManagerThread(void *arg)
{
    threadpool_t *pool = static_cast<threadpool_t *>(arg);
    while (!pool->shutdown)
    {
        // 定时管理线程池
        sleep(DEFAULT_TIME);
        pthread_mutex_lock(&(pool->lock));
        // 任务队列长度
        int queue_size = pool->queue_size;
        // 存活线程数量
        int live_thr_num = pool->live_thr_num;
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->thread_counter));
        // 工作线程数量
        int busy_thr_num = pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->thread_counter));

        // 创建新线程 算法：任务数量 > 最小线程池个数，且存活的线程数 < 最大线程数
        if (queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num)
        {
            cout << "try to create new threads" << endl;
            pthread_mutex_lock(&(pool->lock));
            // 一次添加 DEFAULT_THREAD个线程
            int add = 0;
            for (int i = 0; i < pool->max_thr_num && add < DEFAULT_THREAD_VARY && pool->live_thr_num < pool->max_thr_num; i++)
            {
                cout << "create num " << i << endl;
                if (pool->threads[i] == 0 || !pool->is_thread_alive(pool->threads[i]))
                {
                    pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
                    add++;
                    pool->live_thr_num++;
                }
            }
            pthread_mutex_unlock(&(pool->lock));
        }

        // 销毁多余的空闲线程 算法： 忙线程 *2 < 存活的线程  且  存活线程 > 最小线程数
        if ((busy_thr_num * 2) < live_thr_num && live_thr_num > pool->min_thr_num)
        {
            // 一次清理 DEFAULT_THREAD_VARY 个线程，随机
            pthread_mutex_lock(&(pool->lock));
            // 等待销毁的线程
            pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
            pthread_mutex_unlock(&(pool->lock));
            // 通吃处于空闲状态的线程，他们会自动终止
            for (int i = 0; i < DEFAULT_THREAD_VARY; i++)
            {
                pthread_cond_signal(&(pool->queue_not_empty));
            }
        }
    }
    return NULL;
}
