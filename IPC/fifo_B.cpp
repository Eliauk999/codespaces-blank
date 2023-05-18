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

int main(int argc,char* argv[]) {
    int fd = open("myfifo",O_RDONLY);
    while(1){
        printf("%s\n","waiting for writing...");
        sleep(1);
        char buf[1024];
        int n = read(fd,buf,1024);
        if(n==0){
            perror("read over");
            break;
        }
        printf("%s\n",buf);
    }
    close(fd);
    return 0;
}