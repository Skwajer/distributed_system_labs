#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    mqd_t mq;
    char const *mq_name = "/my_message_queue";
    char const *queue_massage = "Hello, this is a message from queue";

    size_t message_len = strlen(queue_massage) + 1;

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = 1024;
    attr.mq_curmsgs = 0;

    pid = fork();
    if (pid == -1)
    {
        perror("fork calling");
        return 1;
    }

    if (pid == 0)
    {
        sleep(1);
        char message_buffer[1024];
        unsigned int priority;

        mq = mq_open(mq_name, O_RDONLY);
        if (mq == (mqd_t) - 1)
        {
            perror("mq_child_open");
            return 3;
        }

        ssize_t bytes_received = mq_receive(mq, message_buffer, sizeof(message_buffer), &priority);
        if (bytes_received == -1)
        {
            perror("mq_child_receive");
            mq_close(mq);
            return 6;
        }

        message_buffer[bytes_received] = '\0';
        printf("message from parent process: %s\n", message_buffer);

        if(mq_close(mq) == -1)
        {
            perror("mq_child_close");
            return 7;
        }
    }

    else 
    {
        mq = mq_open(mq_name, O_CREAT | O_WRONLY, 0644, &attr);

        if (mq == (mqd_t) - 1)
        {
            perror("mq_parent_create");
            return 2;
        }

        if (mq_send(mq, queue_massage, message_len, 0) == -1)
        {
            perror("parent_message_send");
            mq_close(mq);
            return 4;
        }

        if (mq_close(mq) == -1) 
        {
            perror("mq_parent_close");
            return 5;
        }

        wait(NULL);
    }

    return 0;
}