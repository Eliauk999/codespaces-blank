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
// 利用管道父子进程通信 ，实现命令ls | wc -l
// 子进程执行ls  把查到的结果发给父进程，让父亲执行wc -l    
// 这样做的好处在于，读端需要阻塞等待写端写入数据才能继续操作，避免父进程写完之后无所事事，终止进程。
// 这里没改
int main(int argc,char* argv[]) {
    int fd[2];
    int ret = pipe(fd);
    int pid = fork();
    // 子进程
    if(pid == 0){
        close(fd[1]);
        dup2(fd[0],STDIN_FILENO);
        execlp("wc","wc","-l",NULL); 
    }
    else if(pid>0){
        //关闭读端 准备写
        close(fd[0]);
        // 让标准输出重定向到写端 发给子进程
        dup2(fd[1],STDOUT_FILENO);
        execlp("ls","ls",NULL);
    }
    return 0;
}