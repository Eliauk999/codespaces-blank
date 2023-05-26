#include<stdio.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<error.h>
#include<iostream>
#include<errno.h>
using namespace std;

#define MAXLINE 10

int main(){
    int epfd ;
    pid_t pid;
    char buf[MAXLINE]; 
    char ch = 'a';
    // 创建管道 进程间通信
    int pfd[2];
    pipe(pfd);
    pid = fork();
    // 子进程负责写
    if(pid ==0){
        // 关闭读端
        close(pfd[0]);
        // 不停写入数据
        while(1){
            int i=0;
            // "aaaa\n"
            for( i=0;i<MAXLINE/2;i++){
                buf[i]=ch;
            }
            buf[i-1]='\n';
            ch++;
            // bbbb\n
            for(;i<MAXLINE;i++){
                buf[i]=ch;
            }
            buf[i-1]='\n';
            ch++;
            // 管道里是10个字节 aaaa\n bbbb\n
            write(pfd[1],buf,sizeof(buf));
            sleep(2);
        }
        close(pfd[1]);
    }
    else if(pid>0){
        struct epoll_event event;
        struct epoll_event events[10];
        int res ,len;
        close(pfd[1]);

        // create epoll whit 10 notes
        epfd = epoll_create(10);
        // event.events = EPOLLET; // ET边沿触发
        event.events = EPOLLIN; // LT水平触发（默认）
        event.data.fd = pfd[0];
        epoll_ctl(epfd,EPOLL_CTL_ADD,pfd[0],&event);

        while(1){
            res = epoll_wait(epfd,events,10,-1);
            cout<<"res"<<res<<endl;
            if(events[0].data.fd == pfd[0]){
                len = read(pfd[0],buf,MAXLINE/2);
                // cout<<buf<<endl;
                write(STDOUT_FILENO,buf,len);
            }
        }
        close(pfd[0]);
        close(epfd);

    }
}