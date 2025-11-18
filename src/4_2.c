#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct
{
    char const *data;
    int priority;
}Message;

typedef enum 
{
    LOW_PRIORITY = 1,
    MEDIUM_PRIORITY,
    HIGH_PRIORITY
} MessagePriority;

void handle_high_priority(const char* data) {
    printf("HIGH PRIORITY HANDLER: Processing urgent message: '%s'\n", data);
    printf("    -> Immediate action required!\n");
}

void handle_medium_priority(const char* data) {
    printf("MEDIUM PRIORITY HANDLER: Processing normal message: '%s'\n", data);
    printf("    -> Standard processing...\n");
}

void handle_low_priority(const char* data) {
    printf("LOW PRIORITY HANDLER: Processing background message: '%s'\n", data);
    printf("    -> Background task, can wait...\n");
}

void writer(mqd_t mq)
{
    int i;
    Message Messages[] = 
    {
        {"System shutdown requested", HIGH_PRIORITY},
        {"User login successful", MEDIUM_PRIORITY},
        {"Daily backup completed", LOW_PRIORITY},
        {"CPU usage critical: 95%", HIGH_PRIORITY},
        {"New email received", MEDIUM_PRIORITY},
        {"Log file rotated", LOW_PRIORITY},
        {"END", LOW_PRIORITY}
    };
    size_t struct_len = sizeof(Messages) / sizeof(Messages[0]);

    printf("WRITER: Sending %lu messages with different priorities...\n\n", struct_len - 1);
    for (i = 0; i < struct_len; ++i)
    {
        if (mq_send(mq, Messages[i].data,
             strlen(Messages[i].data), Messages[i].priority) == -1)
             {
                perror("mq_send");
             }
    }
}

void reader(mqd_t mq)
{
    while (1)
    {
        char message_buffer[512];
        unsigned int priority;

        ssize_t bytes_received = mq_receive(mq, message_buffer, sizeof(message_buffer), &priority);
        
        if (bytes_received == -1) 
        {
            perror("Reader: mq_receive");
            break;
        }
        message_buffer[bytes_received] = '\0';

        switch (priority) 
        {
            case HIGH_PRIORITY:
            printf("Message with [HIGH PRIORITY]: %s\n", message_buffer);
            break;
            case MEDIUM_PRIORITY:
            printf("Message with [MEDIUM PRIORITY]: %s\n", message_buffer);
            break;
            case LOW_PRIORITY:
            printf("Message with [LOW PRIORITY]: %s\n", message_buffer);
            break;
        }
        if (strcmp(message_buffer, "END") == 0) {break;}
    }
}

int main()
{
    pid_t pid;
    mqd_t mq;
    char const *mq_name = "/my_message_queue";
    char const *queue_massage = "Hello, this is a message from queue";

    size_t message_len = strlen(queue_massage) + 1;

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 512;
    attr.mq_curmsgs = 0;

    pid = fork();
    if (pid == -1)
    {
        perror("fork_calling");
        return 1;
    }

    if (pid == 0)
    {
        sleep(1);

        mq = mq_open(mq_name, O_RDONLY);
        if (mq == (mqd_t) - 1)
        {
            perror("mq_child_open");
            return 3;
        }

        reader(mq);

        if(mq_close(mq) == -1)
        {
            perror("mq_child_close");
            return 7;
        }
    }

    else 
    {
        int writer_status;
        mq = mq_open(mq_name, O_CREAT | O_WRONLY, 0644, &attr);

        if (mq == (mqd_t) - 1)
        {
            perror("mq_parent_create");
            return 2;
        }

        writer(mq);
        
        if (mq_close(mq) == -1) 
        {
            perror("mq_parent_close");
            return 5;
        }

        wait(NULL);

        mq_unlink(mq_name);
    }

    return 0;
}