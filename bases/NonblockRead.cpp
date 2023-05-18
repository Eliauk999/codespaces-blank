#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>

int main() {
    char buf[1000];
    int fd ,n;
    fd = open("./test2.cpp",O_RDONLY|O_NONBLOCK);
    if(fd<0){
        perror("open error:");
        exit(1);
    }
    loop:
    n = read(fd,buf,1000);
    if(n<1){
        if(errno!=EAGAIN||errno!=EWOULDBLOCK){ //EAGAIN =EWOULDBLOCK =
            perror("read file error");
            exit(1);
        }
        sleep(30);
        write(STDOUT_FILENO,"try again",sizeof("try again"));
        goto loop;
    }
    write(STDOUT_FILENO,buf,n);
    close(fd);
    return 0;
}