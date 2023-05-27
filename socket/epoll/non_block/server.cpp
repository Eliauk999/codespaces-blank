#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
using namespace std;

// 用于演示et + 非阻塞 SOCKET通讯模型
//  只能接受一个客户端的连接
#define PORT 8000

int main(int argc, char *argv[])
{
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    bzero(&client_addr, addrlen);
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(lfd, (sockaddr *)&client_addr, addrlen);
    listen(lfd, 10);

    int epfd = epoll_create(10);
    epoll_event event, events[10];

    int connfd = accept(lfd, (sockaddr *)&client_addr, &addrlen);

    // 修改为非阻塞套接字  获取套接字已有的属性，加上非阻塞之后再装回去
    int flag = fcntl(connfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(connfd, F_SETFL, flag);

    event.data.fd = connfd;
    event.events = EPOLLIN | EPOLLET; // 边沿触发
    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);

    while (1)
    {
        cout << "waiting." << endl;
        int res = epoll_wait(epfd, events, 10, -1); // 阻塞监听
        if (events[0].data.fd == connfd)
        {
            int len = 0;
            constexpr int MAXLEN = 10;
            char buf[MAXLEN];

            while ((len = read(connfd, buf, MAXLEN / 2)) > 0)
            {
                write(STDOUT_FILENO, buf, len);
            }
        }
    }
    return 0;
}