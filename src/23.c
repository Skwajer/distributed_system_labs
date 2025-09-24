#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_PATH_LEN 512

#define ARGC_ERROR -1
#define FILE_ERROR -2
#define FORK_ERROR -3
#define PIPE_ERROR -4
#define INVALID_ARG -5
#define FUNC_ERROR -6

typedef struct
{
    char file_name[MAX_PATH_LEN];
    int count;
} result;

void fork_bomb(size_t height)
{
    printf("height = %d\n", height);
    printf("fork bomb is activated!\n");

    int fork_status_left, fork_status_right;
    if (height == 0)
    {
        return;
    }

    fork_status_left = fork();
    if (fork_status_left == 0)
    {
        usleep(100000);
        fork_bomb(height - 1);
        exit(0);
    }

    fork_status_right = fork();
    if (fork_status_right == 0)
    {
        usleep(100000);
        fork_bomb(height - 1);
        exit(0);
    }

    wait(NULL);
    wait(NULL);
}

int search_str_in_file(char const *path, char const *str)
{
    FILE *f = NULL;
    char line[MAX_PATH_LEN];
    int count = 0;
    size_t len = strlen(str);
    if ((path == NULL) || (str == NULL))
    {
        return 0;
    }

    f = fopen(path, "r");
    if (f == NULL)
    {
        return 0;
    }

    while (fgets(line, sizeof(line), f)) 
    {
        char *p = line;
        while ((p = strstr(p, str)) != NULL) 
        {
            count++;
            p += len;
        }
    }

    fclose(f);
    return count;
}

int find_str_in_files(char const *file_with_paths, char const *str)
{
    FILE *f_with_paths = NULL;
    FILE *current_file = NULL;
    char curent_path[MAX_PATH_LEN];
    size_t len_str;

    size_t childs_count = 0, files_count = 0;
    int i, str_was_find_flag = 0;
    int fork_status;

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

    while (fgets(curent_path, sizeof(curent_path), f_with_paths))
    {
        curent_path[strcspn(curent_path, "\n")] = '\0';

        fork_status = fork();
        if (fork_status < 0)
        {
            printf("fork is not complete");
            close(pipeds[0]);
            close(pipeds[1]);
            fclose(f_with_paths);
            return FORK_ERROR;
        }

        if (fork_status == 0)
        {
            result finder_result;
            usleep(10000);

            strncpy(finder_result.file_name, curent_path, MAX_PATH_LEN);
            fclose(f_with_paths);
            close(pipeds[0]);
            finder_result.count = search_str_in_file(curent_path, str);
            if (finder_result.count > 0)
            
            write(pipeds[1], &finder_result, sizeof(result));
            close(pipeds[1]);
            exit(0);
        }
            childs_count++;
    }
    close(pipeds[1]);
    int status = 0;
    result result1;

    while (read(pipeds[0], &result1, sizeof(result1)) == sizeof(result1))
    {
        if (result1.count > 0)
        {
            str_was_find_flag = 1;
            printf("\nfile path: %s,", result1.file_name);
            printf(" [%d] substrs\n", result1.count);
        }
    }

    while (wait(&status) > 0);
    close(pipeds[0]);
    fclose(f_with_paths);

    if (str_was_find_flag == 0)
    {
        len_str = strlen(str);
        fork_bomb(len_str);
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