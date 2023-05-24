#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
using namespace std;

struct my_lock
{
    int tickNum;
    pthread_mutex_t mut;
    pthread_mutexattr_t mutAttr;
};

// 父子进程对tickNum同时操作，父进程+2，子进程+1

int main(int argc, char *argv[])
{
    int cnt;
    struct my_lock *ml;
    pid_t pid;
    // 创建共享映射区
    int fd = open("test", O_CREAT|O_TRUNC | O_RDWR,0777);
    if (fd < 0)
    {
        perror("open file err:");
        exit(1);
    }
    cout << "fd:" << fd << endl;
    cout << sizeof(my_lock) << endl;
    int ret = ftruncate(fd, sizeof(my_lock));
    if (ret == -1)
    {
        perror("truc err");
        cout << "fd:" << fd << endl;
    }

    ml = (my_lock *)mmap(NULL, sizeof(my_lock), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ml == MAP_FAILED)
    {
        perror("mmap err");
        cout << "fd:" << fd << endl;
    }
    close(fd);
    // 文件连接计数为0时删除文件 （!=引用计数）
    // 在文件打开的情况下unlink()并不会立即删除，并且对文件依然可以进行读写操作，在进程结束之后文件就会被删除掉
    unlink("test");

    // 初始化锁属性
    pthread_mutexattr_init(&ml->mutAttr);
    // 设置属性为进程间共享
    pthread_mutexattr_setpshared(&ml->mutAttr, PTHREAD_PROCESS_SHARED);
    // 初始化互斥锁
    pthread_mutex_init(&ml->mut, &ml->mutAttr);

    pid = fork();
    if (pid == 0)
    {
        for (int i = 0; i < 10; i++)
        {
            pthread_mutex_lock(&ml->mut);
            (ml->tickNum)++;
            cout << "child tickNum = " << ml->tickNum << endl;
            pthread_mutex_unlock(&ml->mut);
            sleep(1);
        }
    }
    else
    {
        for (int i = 0; i < 10; i++)
        {
            pthread_mutex_lock(&ml->mut);
            (ml->tickNum) += 2;
            cout << "Father tickNum = " << ml->tickNum << endl;
            pthread_mutex_unlock(&ml->mut);
            sleep(1);
        }
        wait(0);
    }
    pthread_mutexattr_destroy(&ml->mutAttr);
    pthread_mutex_destroy(&ml->mut);
    return 0;
}