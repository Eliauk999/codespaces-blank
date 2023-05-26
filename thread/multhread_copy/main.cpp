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
#include <sys/mman.h>
using namespace std;
// 用多线程实现对文件的拷贝
char *ptr = nullptr;

struct pos
{
    int blockSize; // 文件长度
    int offset;    // 举例文件首地址的距离
};

void *fun(void *arg)
{
    struct pos *p = (pos *)arg;
    char buf[2048];
    bzero(buf, 2048);
    // 打开fd
    int fd = open("./desc.jpg", O_RDWR | O_CREAT, 0664);
    // 调整读写位置
    lseek(fd, p->offset, SEEK_SET);
    // 把mmap的数据写到fd里
    write(fd, ptr + p->offset, p->blockSize);
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    // 打开源文件
    int fd = open("src.jpg", O_RDWR);
    if (fd < 0)
    {
        perror("open err");
    }
    // 获取文件长度
    int len = lseek(fd, 0, SEEK_END);
    cout << len << endl;
    // 建立映射区用于线程通信
    ptr = (char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("MMAR\n:");
    }
    // 总共五个线程,前四个大小相等,第五个取剩下的尾巴
    for (int i = 0; i < 4; i++)
    {
        sleep(1);
        // 创建结构体 给其赋值 分配各自线程的复制区域
        pos *p = new pos();
        p->blockSize = len / 5;
        p->offset = i * p->blockSize;
        // 创建线程
        pthread_t pid;
        pthread_create(&pid, 0, fun, (void *)p);
        pthread_join(pid, nullptr);
    }
    // 处理尾部数据
    struct pos *p;
    p = (struct pos *)malloc(sizeof(p));
    p->blockSize = len / 5;
    p->offset = p->blockSize * 4;
    p->blockSize = len - p->offset;
    pthread_t tid;
    pthread_create(&tid, NULL, fun, (void *)p);
    pthread_join(tid, NULL);
    close(fd);
    // 释放映射区
    int ret = munmap(ptr, len);
    if (ret == -1)
    {
        perror("munmap\n");
    }
    printf("done\n");
    return 0;
}