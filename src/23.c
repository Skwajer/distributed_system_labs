#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define ARGC_ERROR -1
#define FILE_ERROR -2
#define FORK_ERROR -3
#define PIPE_ERROR -4
#define INVALID_ARG -5
#define FUNC_ERROR -6

typedef struct
{
    char file_name[BUFSIZ];
    int count;
} result;

int search_str_in_file(char const *path, char const *str)
{
    FILE *file = NULL;
    if (path == NULL)
    {
        return -5;
    }

    file = fopen(path, "r");
    if (!file)
    {
        return FILE_ERROR;
    }

    char line[BUFSIZ];
    int count = 0;
    size_t len = strlen(str);

    while (fgets(line, sizeof(line), file)) 
    {
        char *p = line;
        while ((p = strstr(p, str)) != NULL) 
        {
            count++;
            p += len;
        }
    }

  fclose(file);
    return count;
}

int find_str_in_files(char const *file_with_paths, char const *str)
{
    FILE *f_with_paths = NULL;
    FILE *current_file = NULL;
    char *curent_path = NULL;
    size_t current_path_len = 0;
    int fork_status;
    result finder_result;

    f_with_paths = fopen(file_with_paths, "r");
    if (f_with_paths == NULL)
    {
        return FILE_ERROR;
    }

int pipeds[2];
if (pipe(pipeds) == -1)
{
    return PIPE_ERROR;
}

    while (getline(&curent_path, &current_path_len, f_with_paths) != -1)
    {
        fork_status = fork();
        if (fork_status < 0)
        {
            printf("for is not complete");
            close(pipeds[0]);
            close(pipeds[1]);
            return FORK_ERROR;
        }

        if (fork_status == 0)
        {
            close(pipeds[0]);
            if ((finder_result.count = search_str_in_file(curent_path, str) < 0))
            {
                return FUNC_ERROR;
            }
            if (finder_result.count > 0)
            {
                write(pipeds[1], &finder_result, sizeof(int));
                close(pipeds[1]);
                exit(0);
            }
        }   
        free(curent_path);
    }
    close(pipeds[1]);

    int result;

    while (read(pipeds[0], &result, sizeof(result)))
    {
        printf("%s", finder_result.file_name);
    }

    return 0;
}


int main(int argc, char *argv[])
{
    int status;
    if (argc < 3)
    {
        return ARGC_ERROR;
    }
    
    status = find_str_in_files(argv[1], argv[2]);

    return 0;
}