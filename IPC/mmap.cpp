#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>
#include<string.h>
// #include<mmap.h>
#include <sys/mman.h>
#include<iostream>
using namespace std;

int main(int argc,char* argv[]) {
    //注意赋予读写权限
    int fd = open("test",O_CREAT|O_TRUNC|O_RDWR,0664);
    if(fd < 0 ){
        perror("open err");
    }
    printf("%d\n",fd);
    // 由于文件test可能不存在，大小为0，由lseek打开文件并写入数据，大小为11 
    lseek(fd,100,SEEK_END);
    write(fd,"/0",1);
    // int len = lseek(fd,0,SEEK_END);

    //将空文件大小扩充为100
    // int ret = ftruncate(fd,10);
    // if(ret == -1){
    //     perror("truc err");
    // }
    int len = lseek(fd,0,SEEK_END); 
    cout<<"len:"<<len<<endl;
    printf("%d\n",len);
    char* p = (char*) mmap(NULL,len,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
    if(p == MAP_FAILED){
        perror("mmap err");
    }

    // 使用指针对文件进行操作
    strcpy(p,"hella");
    printf("mmap:%s\n",p+110);
    // 释放映射区
    munmap(p,len);
    
    return 0;
}