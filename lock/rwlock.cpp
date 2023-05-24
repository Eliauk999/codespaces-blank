#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
using namespace std;

int count;
pthread_rwlock_t rwlock;

void *fwrite(void *arg)
{
    int t;
    int i = (int)arg;

    while (1)
    {
        t = count;
        usleep(1000);
        pthread_rwlock_wrlock(&rwlock);
        printf("=======write %d: %lu: counter=%d ++counter=%d\n", i, pthread_self(), t, ++count);
        pthread_rwlock_unlock(&rwlock);
        usleep(5000);
    }
    return NULL;
}

void *fread(void *arg)
{
    int i = (int)arg;
    while (1)
    {
        pthread_rwlock_rdlock(&rwlock);
        printf("----------------------------read %d: %lu: %d\n", i, pthread_self(), count);
        pthread_rwlock_unlock(&rwlock);

        usleep(1000);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int i;
    pthread_t tid[8];
    pthread_rwlock_init(&rwlock, NULL);
    // 3个生产者
    for (i = 0; i < 3; i++)
        pthread_create(&tid[i], NULL, fwrite, (void *)i);

    // 5个消费者
    for (i = 0; i < 5; i++)
        pthread_create(&tid[i + 3], NULL, fread, (void *)i);
    // 回收所有线程 
    for (i = 0; i < 8; i++)
        pthread_join(tid[i], NULL);

    // 释放读写琐
    pthread_rwlock_destroy(&rwlock);

    return 0;
}