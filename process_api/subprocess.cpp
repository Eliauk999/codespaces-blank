#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
int main(void)
{
    int i;
    pid_t pid;
    printf("xxx\n");
    
    for(i = 0; i<6; i++){
        pid = fork();
        if(pid == -1){
            perror("fork error;");
            exit(1);
        }else if(pid == 0){
            break;
        }else{
            printf("parent\n");
        }
    }
    printf("===========pid = %u\n",pid);
    if(i < 6){
        sleep(i);
        printf("I am %d child , i=%d,pid = %u\n",i+1,i,getpid());
    }else{
        sleep(i);
        printf("I am parent\n");
    }
    return 0;
}
