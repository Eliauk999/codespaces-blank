#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<fcntl.h>
#include<unistd.h>
#include<pthread.h>

int main(int argc ,char*argv[]){
    char buf[1024];
    int n = 0;
    int fd1 = open(argv[1],O_RDONLY);//read
    int fd2 = open(argv[2],O_RDWR|O_CREAT|O_TRUNC,0664);
    // 等于0 时 说明文件读完了
    while((n = read(fd1,buf,1024))!=0){
        write(fd2,buf,n);
    }
    close(fd1);
    close(fd2);
    return 0;
}