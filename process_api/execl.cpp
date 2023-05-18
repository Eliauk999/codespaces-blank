
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
 
int main(void)
{
	pid_t pid;
	int i;
	for (i = 0; i < 3; i++)
	{
		pid = fork();
        /*
        这个地方要判断pid是否为0是因为fork函数的实现原理，fork函数最后的return 0是子进程进行 
        的，所以进入这个判断的是子进程，而子进程返回的pid就是0，如果这个地方不加上该判断，子进 
        程也会进入该for循环来创造进程，子又生孙孙又生子，而我们只希望父进程来创建三个子进程， 
        所以加上了该判断
        */
		if (pid == 0)
		{
			break;
		}
	}
    /*
    首先父进程进入下面的三个判断，因为父进程pid大于0，所以会进入第一个判断，打印出父进程的 
    pid，然后我们用while循环一直sleep（1）来阻塞父进程，让子进程进入三个判断，因为子进程的pid 
    是0，所以会进入第二个判断，第一个子进程先进入判断，进入if（i == 0）用execl函数重载来实现功 
    能，firefox是命令标识符，www.baidu.com是参数，后面执行同样的步骤，也是父进程先进入判断， 
    之后两个进程分别进入判断并使用execl函数重载来实现功能
    */
	if (pid > 0)
	{
		printf("parent pid %d\nsleeping..\n", getpid());
		while (1)
		{
			sleep(1);
		}
	}
	else if (pid == 0)
	{
		if (i == 0)
		{
			printf("child no.%d pid %d exec firefox..\n", i, getpid());
			execl("/usr/bin/firefox", "firefox", "www.baidu.com", NULL);
		}
		if (i == 1)
		{
			printf("child no.%d pid %d touch files..\n", i, getpid());
			execl("/usr/bin/touch", "touch", "huala", NULL);
		}
		if (i == 2)
		{
			printf("child no.%d pid %d exec ls -l..\n", i, getpid());
			execl("/bin/ls", "ls", "-l", NULL);
		}
	}
 
	return 0;
}