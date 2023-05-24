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

int num =0;
pthread_mutex_t lock;

void * fun(void*arg){
    //将自己设为分离态，不用其他人回收
    pthread_detach(pthread_self());
    int flag = 1000;
    int tem;
    for(int i=0;i<flag;i++){
        // 加锁
        pthread_mutex_lock(&lock);
        // 模拟操作
        tem = num;
        printf("thread %ld ++ = %d\n",pthread_self(),++tem);
        num = tem;
        // 解锁
        pthread_mutex_unlock(&lock);
    }
    pthread_exit(0);
}

int main(int argc,char* argv[]) {
    pthread_t tids[2];
    pthread_mutex_init(&lock,NULL);
    for(int i=0;i<2;i++){
        pthread_create(&tids[i],0,fun,0);
    }
    while(1){
        sleep(1);
    }
    pthread_mutex_destroy(&lock);
    
    return 0;
}