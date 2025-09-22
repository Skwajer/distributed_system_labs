#include <unistd.h>
#include <stdio.h>

int main()
{
    fork();
    printf("Hi\n");
    fork();
    printf("Hi\n");
    return 0;
}