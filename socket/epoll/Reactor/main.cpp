#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
using namespace std;
// epool基于非阻塞io事件驱动
// epool + et + nonblock + call_back

#define MAX_EVENTS 1024
#define BUFLEN 4096
#define SERV_PORT 8008

// 描述文件描述符相关信息
struct myevent_s
{
    int fd;
    int events;
    void *arg;
    void (*call_back)(int fd, int events, void *arg);
    int status; // 是否在监听红黑树上，1表示在
    char buf[BUFLEN];
    int len;
    long last_active; // 记录每次加入到红黑树的时间
};

// 监听红黑树的根节点
int epfd;
// 结构体数组
struct myevent_s gevents[MAX_EVENTS + 1];

// 接受数据
void recvdata(int fd, int events, void *arg);
// 发送数据
void senddata(int fd, int events, void *arg);
// 初始化自定义结构体
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg);
// 把fd和对应事件挂载到监听树上
void eventadd(int efd, int events, myevent_s *ev);
// 建立与客户端的连接并监听
void acceptconn(int lfd, int events, void *arg);
// 监听初始化
void initListenSocket(int efd, int port);
// 删除监听树上的指定节点
void eventdel(int efd, myevent_s *ev);

// 给自定义的结构体初始化
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg)
{
    ev->fd = fd;
    ev->arg = arg;
    ev->events = 0;
    ev->call_back = call_back;
    memset(ev->buf, 0, sizeof(ev->buf));
    ev->len = 0;
    ev->last_active = time(NULL);
    return;
}

// 把fd和对应事件挂载到监听树上
void eventadd(int efd, int events, myevent_s *ev)
{
    struct epoll_event epv = {0, {0}};
    epv.data.ptr = ev;
    epv.events = ev->events = events; // EPOLLIN  或者   EPOLLOUT
    // 将要执行的操作
    int op;
    // 如果不在红黑树里，把他添加进去
    if (ev->status == 0)
    {
        op = EPOLL_CTL_ADD;
        ev->status = 1;
    }
    // 实际添加修改
    if (epoll_ctl(efd, op, ev->fd, &epv) < 0)
    {
        printf("event add failed [fd = %d],events [%d] \n", ev->fd, events);
    }
    else
    {
        printf("event add succeed [fd = %d],events [%d], op = %d \n", ev->fd, events, events);
    }
    return;
}

// 建立与客户端的连接
void acceptconn(int lfd, int events, void *arg)
{
    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);
    int cfd = accept(lfd, (sockaddr *)&cin, &len);
    if (cfd == -1)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
            // 忽略
        }
        printf("%s : accept error%s\n", __func__, strerror(errno));
        return;
    }
    int i = 0;
    do
    {

        for (i = 0; i < MAX_EVENTS; i++)
        {
            if (gevents[i].status == 0)
            {
                break;
            }
        }
        if (i == MAX_EVENTS)
        {
            cout << __func__ << ": max connect limit " << i << endl;
        }
        int flag = fcntl(cfd, F_SETFL, O_NONBLOCK);
        if (flag < 0)
        {
            cout << "fcntl set nonblock err" << endl;
            break;
        }
        eventset(&gevents[i], cfd, recvdata, &gevents[i]);
        eventadd(epfd, EPOLLIN, &gevents[i]);
    } while (0);
    printf("new connect [%s : %d][time :%ld],pos[%d]\n",
           inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), gevents[i].last_active, i);
    return;
}

void recvdata(int fd, int events, void *arg)
{
    myevent_s *ev = (myevent_s *)arg;
    // 读文件描述符写入结构体的缓冲区
    int len = recv(fd,ev->buf,sizeof(ev->buf),0);
    // 将节点从红黑树上摘除
    eventdel(epfd,ev);
    if(len >0){
        ev->len =len;
        // 安全起见给数组末端加个结尾
        ev->buf[len] = '\0';
        // 设置该fd对应的回调函数为senddata
        eventset(ev,fd,senddata,ev);
        // 将fd加入监听树 监听其写事件
        eventadd(epfd,EPOLLOUT,ev);
    }
    else if(len ==0){
        close(ev->fd);
        // ev - gevents 地址相减获得便宜元素位置
        printf("[fd = %d] pos[%ld], closed\n",fd,ev-gevents);
    }
    else{
        close(ev->fd);
        printf("recv[fd = %d] err%s\n",fd,strerror(errno));
    }
    return ;
}

void senddata(int fd,int evnets,void*arg){
    myevent_s *ev = (myevent_s*)arg;
    // 把recvdata收到的数据原封不动发回去
    int len = send(ev->fd,ev->buf,ev->len,0);
    // 从监听树中摘除
    eventdel(epfd,ev);
    if(len>0){
        printf("send [fd = %d] %s\n",fd,ev->buf);
        // 改成写事件再加回去
        eventadd(epfd,EPOLLIN,ev);
    }
    else{
        close(ev->fd);
        printf("send[fd = %d] err%s\n",fd,strerror(errno));
    }
    return;
}

void initListenSocket(int efd, int port)
{
    sockaddr_in sin;
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    // 设置socket为非阻塞
    fcntl(lfd, F_SETFL, O_NONBLOCK);
    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = INADDR_ANY;

    bind(lfd, (sockaddr *)&sin, sizeof(sin));
    listen(lfd, 100);
    // 把lfd放到监听队列的最后一个
    eventset(&gevents[MAX_EVENTS], lfd, acceptconn, &gevents[MAX_EVENTS]);
    eventadd(efd, EPOLLIN, &gevents[MAX_EVENTS]);
    return;
}

// 从epoll监听红黑树中删除一个fd
void eventdel(int efd, myevent_s *ev)
{
    epoll_event epv = {0, {0}};
    // 无效结构体无需删除
    if(ev->status != 1){
        return;
    }
    epv.data.ptr = NULL;
    ev->status = 0;
    // 从监听树上摘除
    epoll_ctl(epfd,EPOLL_CTL_DEL,ev->fd,&epv);
    return;
}


int main(int argc, char *argv[])
{
    unsigned short port = SERV_PORT;
    // 当以命令行启动时，可以自定义监听端口
    if (argc == 2)
    {
        // 启动时自己设置端口
        port = atoi(argv[1]);
    }
    epfd = epoll_create(MAX_EVENTS + 1);
    if (epfd <= 0)
    {
        printf("create efd in %s err %s\n", __func__, strerror(errno));
    }
    // 初始化监听套接字
    initListenSocket(epfd, port);
    // 保存已经满足就绪事件的fd数组
    epoll_event events[MAX_EVENTS + 1];

    cout << "server is running on port " << port << endl;
    // 记录检测到的监听事件的个数。i会重置，这个不会，避免重复检测前100个事件的状态
    int checkpos = 0;
    // 超时验证，删除掉长时间无操作的连接
    while (1)
    {
        // 每次测试100个客户端连接，如果60秒内没有和服务器通信，关闭次连接
        long now = time(0);
        for (int i = 0; i < 100; i++,checkpos++)
        {
            // 如果监听完一轮了，清零重新开始
            if (checkpos == MAX_EVENTS)
            {
                checkpos = 0;
            }
            // 如果不在监听树上 忽略
            if (gevents[checkpos].status == 0)
            {
                continue;
            }
            // 记录访问的间隔时常
            long duration = now - gevents[checkpos].last_active;
            if (duration >= 60)
            {
                close(gevents[checkpos].fd);
                printf("fd = %d timeout\n", gevents[checkpos].fd);
                eventdel(epfd, &gevents[checkpos]);
            }
        }
        // 监听红黑树，将满足的事件fd添加到events数组中，1秒没有事件满足时返回0
        int nfd = epoll_wait(epfd, events, MAX_EVENTS + 1, 1000);
        if (nfd < 0)
        {
            cout << "epoll wait err" << endl;
            break;
        }
        for (int i = 0; i < nfd; i++)
        {
            struct myevent_s *ev = (myevent_s *)events[i].data.ptr;
            // 处理读事件
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
            {
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
            // 处理写事件
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
            {
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
        }
    }
    return 0;
}