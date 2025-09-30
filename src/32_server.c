#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#define FIFO_NAME "ofiles/task2fifo"

#define STRSIZ 101

int main()
{
    int lenghts[101] = {0};
    char str_buffer[STRSIZ];
    size_t same_len_counter;
    int bytes_read, fifo_fd;
    size_t len_str;

    if (mkfifo(FIFO_NAME, 0777) == -1)
    {
        perror("fifo making is failed");
        exit(1);
    }

    fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1)
    {
        perror("fifo files did't open");
        unlink(FIFO_NAME);
        exit(1);
    }

    while((bytes_read = read(fifo_fd, str_buffer, STRSIZ)) > 0)
    {
        str_buffer[bytes_read] = '\0';
        if (str_buffer[bytes_read - 1] == '\n')
        {
            str_buffer[bytes_read - 1] = '\0';
        }

        len_str = strlen(str_buffer);
        if ((len_str > 0) && (len_str < 101))
        {
            lenghts[len_str]++;
        }

        if (lenghts[len_str] >= 5)
        {
            printf("str \"%s\" [len = %lu] was read %d times", str_buffer, len_str, lenghts[len_str]);
            close(fifo_fd);
            unlink(FIFO_NAME);
            exit(0);
        }
    }

    return 0;
}