#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <iostream>
#include<string.h>
#include <dirent.h>
using namespace std;

// 仿写ls -R 递归遍历目录

void isFile(char *name);
void read_dir(char *dir,void(*func)(char*));

// 处理目录
void read_dir(char *dir , void(*func)(char*))
{
    char path[256];         // 文件路径最长256
    DIR *dp = opendir(dir); //表示一个目录流
    struct dirent *sdp;     //表示目录流中的一个目录项
    if (dp == NULL)
    {
        perror("opendir error");
        return;
    }
    // 读取目录项
    while ((sdp = readdir(dp)) != NULL)
    {
        //忽略本级目录文件和上级目录文件
        if(strcmp(sdp->d_name,".")==0||strcmp(sdp->d_name,"..")==0){
            continue;
        }
        // 目录项本身不能被直接访问。拼接完整路径=目录 + 目录项
        sprintf(path,"%s/%s",dir,sdp->d_name);
        // 判断文件类型， 如果是目录，递归遍历，如果是文件，输出属性
        isFile(path);
        (*func)(path);//  func(path);
        
    }
    return;
}

void isFile(char *name)
{
    struct stat sb;
    // 获取该文件的属性
    int ret = stat(name, &sb);
    if (ret == -1)
    {
        perror("stat error");
        return;
    }

    // 如果是目录文件
    if (S_ISDIR(sb.st_mode))
    {
        read_dir(name,isFile);
    }
    //除了目录文件，全都当成普通文件。输出文件大小和名字
    printf("%8ld\t %s\n", sb.st_size, name);
    return;
}


int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        // 如果不传路径，默认遍历当前路径下
        isFile(".");
    }
    else
    {
        isFile(argv[1]);
    }
    return 0;
}