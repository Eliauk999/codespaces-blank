#include <stdio.h>
#include <signal.h>
#include <unistd.h>

//该程序会阻塞 SIGINT 信号，并每隔 1 秒输出进程中被阻塞的未决信号集合。
//因此，如果在该程序运行时按下 Ctrl+C（发送 SIGINT 信号），则会看到在输出中看到该信号被阻塞了。

// 打印出信号集内的信号集合情况
void printsigset(sigset_t *set)
{
    int i;
    for (i = 1; i < 32; i++)
    {
        if (sigismember(set, i)) // 对输入的信号集合进行判断，看第 i 个位置是否为 1，若是则为真并输出，否则为假并输出
            putchar('1');
        else
            putchar('0');
    }
    puts("");
}

int main()
{
    sigset_t s; // 定义信号集合 s（不为空）
    sigset_t p; // 定义信号集合 p（用于接收挂起信号，不为空）

    sigemptyset(&s);                  // 将信号集合 s 清空
    sigaddset(&s, SIGINT);            // 将 SIGINT 加入到信号集合 s 中
    sigprocmask(SIG_BLOCK, &s, NULL); // 将信号集合 s 中的信号加入到进程的信号掩码中，阻塞对应的信号，NULL 表示不需要一个返回值

    while (1)
    {                    // 无限循环
        sigpending(&p);  // 获取当前未决（挂起）的信号，即获取当前进程被阻塞信号集合中未决的信号集合
        printsigset(&p); // 打印出未决信号集合的情况
        sleep(1);        // 等待 1 秒
    }

    return 0;
}