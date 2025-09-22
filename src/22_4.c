#include <unistd.h>
#include <stdio.h>

int main()
{
    if (fork() && (!fork()))
    {   
        if (fork() || fork()) fork();
    }
    printf("52\n");
    return 0;
}