#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_STR_LEN 512

#define ARGC_ERROR -1
#define FILE_ERROR -2
#define FORK_ERROR -3
#define PIPE_ERROR -4
#define INVALID_ARG -5
#define FUNC_ERROR -6

int main()
{
    int pipefd[2];
    pid_t pid;
    char s_for_parent_w[MAX_STR_LEN];
    char *s_for_child_r = "massage from parent";
    size_t child_str_len = strlen(s_for_parent_w);
    
    if (pipe(pipefd) == -1)
    {
        perror("function pipe() is failed");
        return PIPE_ERROR;
    }

    pid = fork();
    if (pid == 0)
    {
        close(pipefd[1]);
        read(pipefd[0], s_for_child_r, child_str_len);
        printf("child received : %s\n", s_for_child_r);
        close(pipefd[0]);
    }
    
    else 
    {
        close(pipefd[0]);
        write(pipefd[1], s_for_parent_w, sizeof(s_for_parent_w));
        close(pipefd[1]);
    }
    return 0;
}