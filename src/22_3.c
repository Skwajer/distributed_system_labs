#include <unistd.h>
#include <stdio.h>

int main()
{
    if (fork() || fork()) fork();
    printf("forked! %d", fork());
    return 0;
}