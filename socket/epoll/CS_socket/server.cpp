#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
// #include
using namespace std;
#define SERVER_PROT 8000
int main(int argc, char *argv[])
{
    int listenfd, connfd;
    char client_ip[BUFSIZ]; /*#define INET_ADDRSTRLEN 16*/
    struct sockaddr_in client_addr, server_addr;
    socklen_t client_addr_len = sizeof(sockaddr_in);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // 设置端口复用 opt = 1表示可复用
    int opt = 1;
    int ret= setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(ret == 0){
        cout<<"端口复用设置成功"<<endl;
    }
    // 监听
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PROT);
    ret = bind(listenfd,(struct sockaddr*)&server_addr,client_addr_len);
    if(ret<0){
        cout<<"bind err"<<endl;
        perror(">");
    }
    ret = listen(listenfd, 10);
    if(ret <0){
        cout<<"listren err"<<endl;
    }
    // 创建epoll
    int epfd = epoll_create(1024);
    // 设置结点属性
    epoll_event event;
    event.data.fd = listenfd;
    event.events = EPOLLIN;
    // 把listenfd添加到监听队列里
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);
    // 创建返回值接受数组
    epoll_event events[1024];
    int cnt = 0;
    while (1)
    {

        // 阻塞等待
        int ret = epoll_wait(epfd, events, sizeof(events) / sizeof(events[0]), -1);
        if (ret < 0)
        {
            perror("epoll err:");
            exit(0);
        }
        for (int i = 0; i < ret ; i++)
        {
            if (events[i].events & EPOLLOUT)
                continue;
            // 处理连接
            if (events[i].data.fd == listenfd)
            {
                connfd = accept(listenfd, (sockaddr *)&client_addr, &client_addr_len);
                int c_port = ntohs(client_addr.sin_port);
                char c_ip[16] = "";
                inet_ntop(AF_INET, (void *)&client_addr.sin_addr.s_addr, c_ip, 16);
                printf("%s , %d\n", c_ip, c_port);
                printf("clientfd:%d No:%d\n", connfd, ++cnt);
                // 把通讯socket装入监听队列
                event.data.fd = connfd;
                event.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);
            }
            else
            {
                char buf[1024];
                int tfd = events[i].data.fd;
                int n =read(tfd, buf, 1024);
                if(n<0){
                    perror("client read err:");
                    epoll_ctl(epfd,EPOLL_CTL_DEL,tfd,NULL);
                    close(tfd);
                }
                else if(n==0){
                    cout<<"client terminal"<<endl;
                    epoll_ctl(epfd,EPOLL_CTL_DEL,tfd,NULL);
                }
                else{
                    printf("%s\n", buf);
                }
            }
        }
    }

    return 0;
}