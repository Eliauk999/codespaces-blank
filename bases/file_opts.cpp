#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include<iostream>
using namespace std;
int main(){
    umask(0000);
    int fd =open("test.cpp",O_CREAT|O_RDWR,0664);
    write(fd,"hello",sizeof("hello")-1);
    char *buf=new char[100];
    lseek(fd,0,SEEK_SET);
    read(fd,(void*)buf,100); 
    cout<<buf<<endl;
    close(fd);
}