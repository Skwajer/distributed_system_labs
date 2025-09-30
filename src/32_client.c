#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define FIFO_NAME "ofiles/task2fifo"
#define STRSIZ 101

int main()
{

    int fifo_fd;
    char str_buffer[102];
    srand(time(NULL));

    fifo_fd = open(FIFO_NAME, O_WRONLY);
    if (fifo_fd == -1)
    {
        perror("fifo file was not opened by the client");
        exit(1);
    }

    while(1)
    {
        size_t rand_strlen = rand() % STRSIZ + 1;
        memset(str_buffer, 'Z', rand_strlen);

        if (write(fifo_fd, str_buffer, rand_strlen) == -1)
        {
            perror("writing error");
            break;
        }

        printf("str with len of %lu was written in fifo file\n", rand_strlen);
    }

    close(fifo_fd);
    return 0;
}