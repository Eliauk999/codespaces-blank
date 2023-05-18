#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <iostream>
#include<cstring>
using namespace std;
int main()
{
    int fd[2];
    int ret = pipe(fd);
    if(ret == -1){
        perror("pipe error\n");
    }
    int pid = fork();
    if(pid>0){
        close(fd[0]);
        char buf[100];
        //读取终端数据
        read(STDIN_FILENO,buf,sizeof(buf));
        write(fd[1],buf,sizeof(buf));
        close(fd[1]);
    }
    else if(pid ==0){
        close(fd[1]);
        char buf[100];
        ret = read(fd[0],buf,100);
        //输出到终端
        write(STDOUT_FILENO,buf,strlen(buf));
        close(fd[0]);
    }
    return 0;
    
}