#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>


void handler(int signo)
{
    int status;    
    pid_t pid;
    while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status))
            printf("child %d exit %d\n", pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("child %d cancel signal %d\n", pid, WTERMSIG(status));
    }
}

int main()
{
    pid_t pid;
    int i = 0;

    //循环创建子进程
    for(i = 0; i < 10; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            break;
        }
        else if (pid < 0)
        {
            perror("pid error:");
            exit(1);
        }
    }
    if (pid > 0)
    {
        printf("%d\n",i);

        sigset_t set,oldmask;
        sigemptyset(&set);
        sigemptyset(&oldmask);
        sigaddset(&set,SIGCHLD);
        //屏蔽
        sigprocmask(SIG_BLOCK,&set,NULL);
        struct sigaction act;
        //安装信号
        act.sa_flags = 0;
        act.sa_handler = handler;
        sigemptyset(&act.sa_mask);
        //父进程注册信号捕捉函数
        sigaction(SIGCHLD,&act,NULL);

        //开启
        sigprocmask(SIG_SETMASK,&oldmask,NULL);
        while (1);
    }
    else if (pid == 0)
    {
        int n = 1;
        while (n--) {
            printf("child ID %d\n", getpid());
            sleep(1);
        }
        return i + 1;
    }
    return 0;
}
