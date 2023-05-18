#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>
#include<iostream>
#include <signal.h>
using namespace std;
// 试图触发管道报错
int main() {
    int pipefd[2];
    char buf[] = "hello";
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    // fork();
    // 关闭读端口
    close(pipefd[0]);
    // 向写端口写入数据，但是读端口已经关闭
    if (write(pipefd[1], buf, sizeof(buf)) == -1) {
        perror("write");
        // 判断写入失败是否由于SIGPIPE引起的
        if (errno == SIGPIPE) {
            cout << "Caught SIGPIPE signal!" << endl;
        }
        else{
            cout<<"?"<<endl;
        }
    }
    cout<<"good"<<endl;
    // 关闭写端口
    close(pipefd[1]);
    return 0;
}