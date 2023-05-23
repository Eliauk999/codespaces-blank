#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>
#include<iostream>
using namespace std;

void fun(){
    printf("tfunc pid = %d tid = %lu 线程号=%u\n",getpid(),pthread_self(),gettid());
    return ;
}

int main(int argc,char* argv[]) {
    pthread_t tid ;
    

    return 0;
}