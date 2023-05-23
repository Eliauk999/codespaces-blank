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

void * fun(void*p){
    //让线程死循环 不退出
    while (1)
    {
        sleep(1);
    }
    pthread_exit(0);
}
int count=0;
int main(int argc,char* argv[]) {
    int err = 0;
    pthread_t tid;
    cout<<"create ing:"<<endl;
    fork();
    while(err==0){
        err = pthread_create(&tid,NULL,fun,NULL);
        count++;
        printf("count = %d\n",count);
    }
    printf("create thread error :%s\n",strerror(errno));
    printf("max nunmber of thread within a process is %d \n",count);
    cout<<"press entry to continue.."<<endl;
    getchar();
    return 0;
}
