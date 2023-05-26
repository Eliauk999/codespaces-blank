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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/select.h>
using namespace std;

#define SERVER_PROT 8000

int main(int argc, char *argv[])
{
    int listenfd, connfd;
    char client_ip[BUFSIZ]; /*#define INET_ADDRSTRLEN 16*/
    struct sockaddr_in client_addr, server_addr;
    socklen_t client_addr_len;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // 设置端口复用 opt = 1表示可复用
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 监听
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PROT);
    int ret = bind(listenfd,(sockaddr*)&server_addr,client_addr_len);
    listen(listenfd, 10);

    // 设置套接字集合
    fd_set rset, allset; // read set读集合
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset); // 把监听描述符加入集合中
    // 设置监听范围 即最大文件描述符
    int nfds = listenfd + 1;
    // select返回值
    int ret;
    // 接受用户数据的buf
    char recvBuf[1024];
    cout<<"start running."<<endl;
    while (1)
    {
        rset = allset; // allset表示所有想监听的（只针对read事件），rset表示实际监听到的读事件
        ret = select(nfds, &allset, nullptr, nullptr, nullptr);
        // select执行出错
        if (ret < 0)
        {
            perror("select error");
            exit(-1);
        }
        // 如果是lfd被监听到了，说明有客户端来请求连接
        if (FD_ISSET(listenfd, &allset))
        {
            // 建立连接
            client_addr_len = sizeof(client_addr);
            connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
            // 输出对端信息
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, BUFSIZ);
            printf("client ip:%s port:%d ,connect successful\n", client_ip, ntohs(client_addr.sin_port));
            // 把建立好的连接也加入监听集合里
            FD_SET(connfd, &allset);
            // 更新监听范围
            if (connfd >= nfds)
            {
                nfds = connfd + 1;
            }
            // 如果只监听到了lfd，后面就不用执行了，因为属于lfd的工作已经完成了
            if (ret == 1)
                // 结束本次while循环 即跳过下面的for
                continue;
        }
        // 处理监听到的来自客户端的消息
        for (int i = listenfd + 1; i < nfds; i++)
        {
            if (FD_ISSET(i, &rset))
            {
                int n = read(i, recvBuf, sizeof(recvBuf));
                // 读文件出错
                if (n < 0)
                {
                    perror("read err");
                    exit(-1);
                }
                // 已经读完全部数据
                else if (n == 0)
                {
                    // 关闭描述符
                    close(i);
                    // 将其移出监听集合
                    FD_CLR(i, &allset);
                }
                // 正常接受数据
                else
                {
                    // 模拟处理数据，将所有小写转大写 并返回
                    for (int j = 0; j < n; j++)
                    {
                        recvBuf[j] = toupper(recvBuf[j]);
                    }
                    write(i, recvBuf, sizeof(recvBuf));
                    // 输出到终端上
                    write(STDOUT_FILENO, recvBuf, sizeof(recvBuf));
                }
            }
        }
    }
    close(listenfd);

    return 0;
}