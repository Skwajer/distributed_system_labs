#include <bits/pthreadtypes.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

#define MAX_MESSAGE_COUNT 10

typedef struct
{
    char *data;
    int priority;
}Message;

typedef struct 
{
    char guid_v1[37];
    char *data;
    size_t data_len;
    int priority;

    enum Status 
    {
        STATUS_QUEUED,
        STATUS_PROCESSING, 
        STATUS_COMPLETED,
        STATUS_FAILED
    } status;

    char **errors;
    size_t errors_count;

    

}Validation_command;

typedef struct queue_Node
{
    Validation_command *cmd;
    struct queue_Node *prev;
    struct queue_Node *next;
} Node;

typedef struct 
{
    Node *head;      
    Node *tail;
    int count;
    int max_size;
    pthread_mutex_t mutex;
} ValidationQueue;

ValidationQueue validation_queue;

void init_validation_queue(ValidationQueue queue)
{
    queue.head = NULL;
    queue.tail = NULL;
    queue.count = 0;
    queue.max_size = MAX_MESSAGE_COUNT;
    pthread_mutex_init(&queue.mutex, NULL);
}

int enQueue(ValidationQueue *queue, Validation_command *cmd)
{
    pthread_mutex_lock(&queue->mutex);
    if (queue->count == queue->max_size)
    {
        pthread_mutex_unlock(&queue->mutex);
        return 1;
    }

    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL)
    {
        perror("new_node allocation");
        return -1;
    }
    new_node->cmd = cmd;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (queue->count == 0)
    {
        queue->head = new_node;
        queue->tail = new_node;
        queue->count++;
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    Node *iter = queue->head;
    while(iter && iter->cmd->priority >= cmd->priority)
    {
        iter = iter->next;
    }
    
    if (iter == queue->head)
    {
        new_node->next = queue->head->next;
        queue->head->prev = new_node;
        queue->head = new_node;
    }

    else if(iter->next == NULL)
    {
        queue->tail->next = new_node;
        new_node->prev = queue->tail;
        queue->tail = new_node;
    }

    else
    {
        Node *prev_node = iter->prev;
        prev_node->next = new_node;
        new_node->prev = prev_node;
        new_node->next = iter;
        iter->prev = new_node;
    }

    queue->count++;
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

Validation_command *deQueue(ValidationQueue *queue)
{
     pthread_mutex_lock(&queue->mutex);
    
    if (queue->count == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    // Извлекаем из головы (элемент с наивысшим приоритетом)
    Node *temp = queue->head;
    Validation_command *cmd = temp->cmd;
    
    if (queue->count == 1) 
    {
        queue->head = NULL;
        queue->tail = NULL;
    } 

    else 
    {
        queue->head = queue->head->next;
        queue->head->prev = NULL;
    }
    
    free(temp);
    queue->count--;
    
    pthread_mutex_unlock(&queue->mutex);
    return cmd;
}


//TODO: think over the logic of the validation chain (chain of responsibility)
typedef struct Validator
{
    int (*validate) (struct Validator *self, void *command);
    struct Validator *next;

}Validator;

typedef struct Validator_Required_fields    
{


}Validator_Required_fields;

typedef struct Validator_of_Field_type
{

}Validator_of_Field_type;

typedef struct Vlidator_of_acceptable_values
{

}Vlidator_of_acceptable_values;

uint64_t get_guid_v1_timestamp() 
{
    time_t current_time;
    time(&current_time);
    
    const uint64_t seconds_1582_to_1970 = (388ULL * 365ULL + 97ULL) * 24ULL * 3600ULL;
    
    uint64_t time_since_1582 = (uint64_t)current_time + seconds_1582_to_1970;
    
    uint64_t timestamp_100ns = time_since_1582 * 10000000ULL;
    
    timestamp_100ns += (rand() % 10000000);
    
    return timestamp_100ns;
}

void* validation_worker_thread(void* arg) 
{
    Validation_command *current_cmd;
    
    while (1)
    {
        if ((current_cmd = deQueue(&validation_queue)) != NULL) 
        {
            printf("Validating: %s\n", current_cmd->data);
            
            // TODO: handle validating chain
            
            if (current_cmd->data) free(current_cmd->data);
        } 
        
        {
            usleep(100000);
        }
    }
    return NULL;
}

int main()
{
    //TODO: handle create message queue in server process
    mqd_t mq;
    char const *mq_name = "/my_mq";
    char message_buffer[512];
    unsigned int priority;

    mq = mq_open(mq_name, O_RDONLY);
    if (mq == (mqd_t) - 1)
        {
            perror("mq_server_open");
            return 1;
        }

    pthread_mutex_t queue_mutex;

    init_validation_queue(validation_queue);
    pthread_t worker_thread;
    pthread_create(&worker_thread, NULL, validation_worker_thread, NULL);


    pthread_mutex_init(&queue_mutex, NULL);
    while (1)
    {
        ssize_t bytes_received = mq_receive(mq, message_buffer, sizeof(message_buffer), &priority);
        if (bytes_received == -1)
        {
            perror("mq_server_receive");
            continue;
        }
        message_buffer[bytes_received] = '\0';

        Validation_command *cmd_for_validation = (Validation_command *)malloc(sizeof(Validation_command));
        if(cmd_for_validation == NULL)
        {
            continue;
        }

        cmd_for_validation->data = malloc(bytes_received + 1);
        if (cmd_for_validation->data == NULL)
        {
            free(cmd_for_validation);
            continue;
        }
        strcpy(cmd_for_validation->data, message_buffer);
        cmd_for_validation->priority = priority;
        cmd_for_validation->status = STATUS_QUEUED;

        // TODO: fix that - generate GUID_v1
        strcpy(cmd_for_validation->guid_v1, "12345678-1234-1234-1234-123456789012");

        cmd_for_validation->errors = NULL;
        cmd_for_validation->errors_count = 0;
        enQueue(&validation_queue, cmd_for_validation);
    }




    return 0;
}