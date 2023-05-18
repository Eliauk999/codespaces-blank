#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {
    int fd1, fd2;
    char buf[1024];

    // 打开文件，写入文件内容
    fd1 = open("test.txt", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd1 < 0) {
        perror("open file error");
        exit(1);
    }
    write(fd1, "Hello, World!\n", strlen("Hello, World!\n"));
    lseek(fd1, 0, SEEK_SET);

    // 复制文件描述符
    if (fd2 < 0) {
        perror("dup2 error!");
        exit(1);
    }

    // 读取文件内容并输出
    read(fd2, buf, sizeof(buf));
    printf("%s", buf);

    // 关闭文件
    close(fd1);
    close(fd2);
    return 0;
}