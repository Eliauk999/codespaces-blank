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
#include<strings.h>
#include <iostream>
using namespace std;
// epool基于非阻塞io事件驱动
// epool + et + nonblock + call_back

#define MAX_EVENTS 1024
#define BUFLEN 4096
#define SERV_PORT 8008



#include"ThreadPoll.h"
#include<vector>

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

//数据缓存
struct DataBuffer
{
    DataBuffer( int _sock , char* _buf , int _nlen )
        :sockfd(_sock),buf(_buf),nlen(_nlen){}
    int sockfd;
    char* buf;
    int nlen;
};


class Epoll{
public:
    // 监听树根
    int epfd;
    // 监听fd
    int lfd;
    // 线程池
    threadpool_t *pool;
    // 监听树上的事件集合
    struct myevent_s gevents[MAX_EVENTS + 1];
    // 初始化并开始监听
    bool InitListen();
    // while循环
    void EventLoop();
    // 添加 删除节点
    void AddEvent(myevent_s*ev);
    void DelEvent(myevent_s*ev);


    static void  Accept_task(void *arg);
    static void  Recv_task(void *arg);
    static void  Send_task(void *arg);


    // 事件处理 ---- reactor 对应的实际事件。

    // 接受链接  
    void Accept();
    // 接受数据
    void Recv(void *arg);
    // 发送数据
    void Send();


    
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
