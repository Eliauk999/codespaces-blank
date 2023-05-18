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
//想运行需要填命令行参数 为目标文件的路径
int main(int argc,char* argv[]) {
    struct stat sbuf;
    int ret = stat(argv[1],&sbuf);
    if(ret == -1){
        perror("stat:");
    }
    //只输出了文件大小
    printf("st_size:%d\n",sbuf.st_size);
    //判断文件类型，只能一个一个判断。用宏S_ISREG、S_ISLINK ...
    if(S_ISREG(sbuf.st_mode)){
        printf("%s\n","its a regular file");
    }else{
        printf("%s\n","no ieda");
    }
    return 0;
}