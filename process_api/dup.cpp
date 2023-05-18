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
    if(argc == 1) {
        cout<<"missing parameter to dup"<<endl; 
        return -1;
    }
    int fd = open(argv[1],O_RDONLY);
    int newfd = dup(fd);
    // cout<<fd<<newfd<<endl;
    cout<< write(newfd,"111",3);
    return 0;
}