#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{

    // 1.判断文件是否存在
    int ret = access("myfifo", F_OK);
    if (ret == -1)
    {
        printf("管道不存在，创建管道\n");
        ret = mkfifo("myfifo", 0664);
        if (ret == -1)
        {
            perror("mkfifo");
            exit(0);
        }
    }
    printf("%s\n","管道已经存在");
    // 3.以只写的方式打开管道
    int fd = open("myfifo",O_WRONLY);
    char *buf = "hello";
    write(fd,buf,sizeof(buf));
    printf("%s\n","向管道写入数据完毕:");
    printf("%s\n",buf);
    sleep(3);
    //再写一次
    write(fd,buf,sizeof(buf));
        printf("%s\n","向管道写入数据完毕:");
    printf("%s\n",buf);
    printf("%s\n","Done");
    sleep(5);

    close(fd);
    return 0;
}