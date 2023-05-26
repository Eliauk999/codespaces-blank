#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<pthread.h>
#include<string.h>
#include<iostream>
using namespace std;

int main() {
    struct sockaddr_in mine,server;
    server.sin_family=AF_INET;
    server.sin_port=htons(8000);
    inet_aton("172.17.0.1",&server.sin_addr);
    // inet_aton("172.16.5.4",&server.sin_addr);
    int fd = socket(AF_INET,SOCK_STREAM,0);
    int len = sizeof(struct sockaddr_in);
    int flag = connect(fd,(struct sockaddr*)&server,len);
    if(flag!=0)cout<<"err"<<endl;
    // char recvbuf[200]= "";
    // recv( fd, (void *)recvbuf,200, 0);
    // printf("%s\n",recvbuf);
    char sendbuf[200]="localtime\n";
    send( fd,(void *)sendbuf, 200, 0);
    write(STDOUT_FILENO,sendbuf,sizeof(sendbuf));
    
    // char recvtime[200]="";
    recv( fd, (void *)sendbuf,200, 0);
    printf("%s\n",sendbuf);

    return 0;
}