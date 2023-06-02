#include "Reactor.h"
#include "ThreadPoll.h"

class TcpNet
{
public:
    int epfd;
    threadpool_t *pool;
    bool InitNet();
    void UninitNet();
    bool SendData();
    void RecvData();
};

bool TcpNet::InitNet()
{
    unsigned short port = SERV_PORT;
    // 当以命令行启动时，可以自定义监听端口
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
    threadpool_t *thp = threadpool_create(10, 100, 100);
    cout << "threadpool created." << endl;
}

void TcpNet::UninitNet()
{
}

bool TcpNet::SendData()
{
    return false;
}

void TcpNet::RecvData()
{
}
