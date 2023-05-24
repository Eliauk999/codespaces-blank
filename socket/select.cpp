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
#include <sys/socket.h>
#include <netinet/in.h>
#include<time.h>
#include <sys/select.h>
using namespace std;

#define SERVER_PROT 8000

int main(int argc,char* argv[]) {
    int listenfd,connfd;
    char buf[BUFSIZ];
    struct sockaddr_in client_addr,server_addr;
    socklen_t client_addr_len;
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    // 设置端口复用 opt = 1表示可复用
    int opt =1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    // 监听
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PROT);
    listen(listenfd,10);

    // 设置套接字集合
    fd_set rset,allset;
    FD_SET(listenfd,&allset);

    return 0;
}