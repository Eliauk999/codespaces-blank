#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <iostream>
using namespace std;

void catch_child(int signo)
{
    int wpid;
    int status;
    // while((wpid = wait(NULL))!= -1){
    while ((wpid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // 如果是自然死亡的 WIFEXITED值为1
        if (WIFEXITED(status))
            // 获取进程返回值
            printf("catch child id = %d ,thread No.  %d\n", wpid, WEXITSTATUS(status));
        // 如果是被信号杀死的  同理
        else if (WIFSIGNALED(status))
            printf("child %d cancel signal %d\n", wpid, WTERMSIG(status));
    }
    return;
}

int main(int argc, char *argv[])
{

    int pid = 0;
    int i = 0;
    // 给屏蔽字初始化，阻塞SIGCHLD信号，以避免在父进程注册捕捉函数之前，子进程已经死亡，其信号被忽略，变成僵尸进程
    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, &old);

    // 注意i用的是全局变量 不要再声明int
    for (i = 0; i < 10; ++i)
    {
        //  fork() 语句的返回值赋值给 pid ，0是子进程
        if ((pid = fork()) == 0)
        { // 创建多个子线程
            break;
        }
    }

    if (i == 10)
    // if(pid>0)
    {

        struct sigaction act;
        // 结构体初始化
        //  绑定捕捉函数
        act.sa_handler = catch_child;
        // 设置屏蔽字（清空）用于捕捉函数执行期间不被自己的信号中断又能接收到其他信号
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;

        sigaction(SIGCHLD, &act, NULL);

        // 解除对SIGCHLD的屏蔽
        sigprocmask(SIG_SETMASK, &old, NULL);

        printf("%s,%d\n", "here is father", getpid());
        while (1)
        {
            // printf("%s\n","*");
            sleep(1);
        }
    }
    else
    {
        // 子进程执行完自动死亡
        sleep(1);
        printf("here is child pid =%d\n", getpid());
        // 返回自己的排名（第i+1个子进程）
        return i + 1;
    }

    return 0;
}