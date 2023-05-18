#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>
#include<iostream>
using namespace std;

int main(void)
{
	int i = 0;
	// printf("i   son/parent   ppid   pid    ret\n");
	//ppid指当前进程的父进程pid
	//pid指当前进程的pid,
	//ret指fork返回给当前进程的值
	for (i = 0; i<2; i++){
		pid_t ret = fork();
		if (ret == 0)
			printf("%d child  %4d %4d %4d\n", i, getppid(), getpid(), ret);
		else
			printf("%d parent %4d %4d %4d\n", i, getppid(), getpid(), ret);
	}
	return 0;
}
