#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>
#include<iostream>
#include<time.h>
#include<string.h>
using namespace std;

int Daemon_work(){
    int fd ;
    time_t tp;
    char time_buf[1024];
    // 把正常的执行输入进log文件中
    if((fd = open("time.log",O_RDWR|O_CREAT,0664))==-1){
        perror("open failed");  
        exit(0);
    }
    while (1)
    {
        // 每隔三秒写入一次当前时间
        bzero(time_buf,1024);
        tp = time(NULL);
        ctime_r(&tp,time_buf);
        write(fd,time_buf,strlen(time_buf));
        sleep(3);
    }
}

void Daemon_Create(){
    int err = open("ERROR_FILE",O_CREAT|O_RDWR,0664);
    if(err == -1){
                perror("open failed");
        exit(0);
    }
    int pid = fork();
    if(pid == 0){
        setsid();//脱离终端控制
        //关闭无用描述符，把err重定向到错误文件中，遇到报错时写入此文件
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        dup2(err,STDERR_FILENO);
        // 设置文件掩码，以保证能正常运行
        umask(0002);
        chdir("../");//修改工作目录，方便处理系统中的其他进程，一般设置为/
        Daemon_work();//执行任务
    }
    else{
        perror("fork err");
    }
}

int main(int argc,char* argv[]) {
    Daemon_Create();
    return 0;
}