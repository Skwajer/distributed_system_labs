#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_PATH_LEN 1024
#define MAX_TYPE_LEN 16

typedef struct dirent dirent;


#define ARGC_ERROR -1
#define FILE_ERROR -2
#define PIPE_ERROR -3
#define INVALID_ARG -4
#define FUNC_ERROR -5
#define DIR_ERROR -6

void castom_ls(int recmin, int recmax, char const *curent_path)
{
    DIR *dir;
    dirent *entry;
    struct stat statbuf;
    char *type;

    dir = opendir(curent_path);

    char full_path[MAX_PATH_LEN];
    if (dir == NULL)
    {
        return;
    }

    if (recmin == recmax)
    {
        printf("|---%s\n", curent_path);


        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            {
                continue;
            }
            snprintf(full_path, sizeof(full_path), "%s/%s", curent_path, entry->d_name);
            type = strrchr(entry->d_name, '.');
            type == NULL ?
                type = "" : ++type;
                
            if (stat(full_path, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode)) 
                {
                    printf("📄 || name: %s || address: %lu extension: %s\n", entry->d_name, entry->d_ino, type);
                } 
                else if (S_ISDIR(statbuf.st_mode)) 
                {
                    printf("📁 || name: %s || address: %lu %s\n", entry->d_name, entry->d_ino, type);
                }
            }
        }
        closedir(dir);
        return;
    }
        while ((entry = readdir(dir)) != NULL) 
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
            {
                continue;
            }
            
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", curent_path, entry->d_name);
            
            struct stat file_stat;
            if (stat(full_path, &file_stat) == 0 && S_ISDIR(file_stat.st_mode)) 
            {
                castom_ls(recmin + 1, recmax, full_path);
            }
        }
    
    closedir(dir);
}

int main(int argc, char *argv[])
{
    int i;
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
    
    for (i = 3; i < argc; ++i)
    {
        castom_ls(recmin, recmax, argv[i]);
    }


    return 0;
}
