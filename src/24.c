#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_PATH_LEN 1024


#define ARGC_ERROR -1
#define FILE_ERROR -2
#define FORK_ERROR -3
#define PIPE_ERROR -4
#define INVALID_ARG -5
#define FUNC_ERROR -6

void castom_ls(int recmin, int recmax, char const *curent_path)
{
    if (recmin == recmax)
    {
        printf("|---%s", curent_path);
        return;
    }
}

int main(int argc, char *argv[])
{
    int recmin, recmax;
    char full_path[MAX_PATH_LEN];

    if (argc < 4)
    {
        return ARGC_ERROR;
    }

    recmin = atoi(argv[1]);
    recmax = atoi(argv[2]);
    if (recmin < 0 || recmax < 0 || recmax < recmin)
    {
        return INVALID_ARG;
    }
    
    strncpy(full_path, argv[3], MAX_PATH_LEN);

    castom_ls(recmin, recmax, full_path);


}
