#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("current proccess id: %d\n", getpid());
    printf("parent process id: %d\n", getppid());
    printf("process group id: %d\n", getpgrp());
    printf("real user id: %d\n", getuid());
    printf("real calling process group id: %d\n", getgid());
    printf("effective calling process id: %d\n", geteuid());
    printf("effective calling process group id: %d\n", getegid());
    return 0;
}