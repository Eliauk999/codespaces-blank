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

int main(int argc,char* argv[]) {
    int pid = fork();
    if(pid ==0){
        execlp("ls","ls","-l",NULL);
        perror("err");
        exit(0);
    }
    else{
        cout<<"fat"<<endl;
        sleep(1);
    }
    return 0;
}