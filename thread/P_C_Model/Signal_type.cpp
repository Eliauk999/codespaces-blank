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
using namespace std;

// Producer Consumer 单一生产者消费者模型

// 链表作为共享数据，需要用锁保护
struct msg{
    int data;
    struct msg *next;
}node;
// 全局变量
struct msg *head;

// 静态初始 互斥锁和条件变量
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t has_product = PTHREAD_COND_INITIALIZER;

void* consumer(void* arg){
    struct msg *mp;
    for(;;){
        pthread_mutex_lock(&lock);
        // 头指针为空 说明没有节点 无法消费
        while(head==NULL){
            pthread_cond_wait(&has_product,&lock);
        }
        mp = head;
        head = mp->next;
        pthread_mutex_unlock(&lock);
        cout<<"Consumer "<<pthread_self()<<"----"<<mp->data<<endl;
        free(mp);
        sleep(rand()%3);
    }
}

void* producer(void*){
    msg *t ;
    int cnt =0;
    while(1){
        t = new msg();
        t->data = cnt++;
        cout<<"Producer---- "<<pthread_self()<<endl;
        pthread_mutex_lock(&lock);
        t->next =head;
        head = t;
        pthread_mutex_unlock(&lock);
        
        pthread_cond_signal(&has_product);
        sleep(rand()%2);
    }
}

int main(int argc,char* argv[]) {
    pthread_t pid,cid;
    srand(time(NULL));
    pthread_create(&pid,NULL,producer,NULL);
    pthread_create(&cid,NULL,consumer,NULL);

    pthread_join(pid,NULL);
    pthread_join(cid,NULL);

    return 0;
}