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
    int ret = fork();
    if (ret == 0)
    {
        cout << "son" << endl;
        cout << "getpgrp() " << getpgrp() << endl;
        cout << getppid() << endl;
        ret = setsid();
        if (ret == -1)
        {
            cout << "set err" << endl;
        }
        else
        {

            cout<<"pid:" << getpid() << endl;
            cout<<"sid:" << getsid(getpid()) << endl;
            cout <<"parentid:"<< getppid() << endl;
        }
    }
    else
    {

        sleep(3);
        cout << "fa" << endl;

            cout << "getpid():" << getpid() << endl;
        cout << "getpgrp() " << getpgrp() << endl;

setpgrp();
cout<<setpgrp()<<endl;
            cout << "getpid():" << getpid() << endl;
        cout << "getpgrp() " << getpgrp() << endl;
        ret = setsid();
        if (ret == -1)
        {
            cout << " fa set err" << endl;
        }
        else
        {
            cout<<"pid:" << getpid() << endl;
            cout<<"sid:" << getsid(getpid()) << endl;
            cout <<"parentid:"<< getppid() << endl;
        }
    }
    return 0;
}