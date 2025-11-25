#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <sched.h>
#include <stddef.h>
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
    char guid_v1[33];
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

typedef struct Hash_entry
{
    Validation_command *cmd;
    struct Hash_entry *next;
}Hash_entry;

typedef struct guid_table
{
    Hash_entry **buckets;
    size_t capacity;
    size_t size;
    pthread_mutex_t mutex;

} guid_table;

guid_table table;

void init_validation_queue(ValidationQueue queue);
void free_validation_command(Validation_command *cmd);

int enQueue(ValidationQueue *queue, Validation_command *cmd);
Validation_command *deQueue(ValidationQueue *queue);

uint64_t get_guid_v1_timestamp();
void generate_simple_guide_v1(char *guid_buffer);

void *validation_worker_thread(void *arg);
void *send_status_to_client(void *arg);

unsigned long guid_hash(char const *guid_buffer);
int guid_hash_table_init(guid_table *table);
int guid_table_put(guid_table *table, Validation_command *cmd);
Validation_command* guid_table_get(guid_table *table, const char *guid);




ValidationQueue validation_queue;

void init_validation_queue(ValidationQueue queue)
{
    queue.head = NULL;
    queue.tail = NULL;
    queue.count = 0;
    queue.max_size = MAX_MESSAGE_COUNT;
    pthread_mutex_init(&queue.mutex, NULL);
}

// void free_validation_command(Validation_command *cmd)
// {
//     if (!cmd) return;
    
//     free(cmd->data);
    
//     // Освобождаем ошибки если есть
//     if (cmd->errors) {
//         for (size_t i = 0; i < cmd->errors_count; i++) {
//             free(cmd->errors[i]);
//         }
//         free(cmd->errors);
//     }
    
//     free(cmd);
// }

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

void generate_simple_guide_v1(char *guid_buffer)
{
    uint64_t timestamp = get_guid_v1_timestamp();
    uint64_t other_random_part = ((uint64_t)rand() / rand());

    snprintf(
        guid_buffer, 33, "%016llx%016llx",
             (unsigned long long)timestamp,
             (unsigned long long)other_random_part );
}

void *validation_worker_thread(void* arg) 
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

void* status_api_thread(void* arg)
{
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 37;
    attr.mq_curmsgs = 0;
    
    mqd_t status_mq = mq_open("/validation_status", O_CREAT | O_RDONLY, 0644, &attr);
    if (status_mq == (mqd_t)-1) {
        perror("status_api_thread: mq_open");
        return NULL;
    }
    
    printf("Status API thread started. Queue: /validation_status\n");
    
    char guid_buffer[37];
    
    while (1) 
    {
        ssize_t bytes = mq_receive(status_mq, guid_buffer, sizeof(guid_buffer), NULL);
        if (bytes == -1) 
        {
            perror("mq_receive");
            continue;
        }
        
        guid_buffer[bytes] = '\0';
        
        printf("Status request received for GUID: %s\n", guid_buffer);
        
        Validation_command *cmd = guid_table_get(&table, guid_buffer);
        
        char response[256];
        if (cmd == NULL)
        {
            snprintf(response, sizeof(response), "NOT_FOUND");
        } 

        else 
        {
            switch (cmd->status)
            {
                case STATUS_QUEUED:
                    snprintf(response, sizeof(response), "QUEUED");
                    break;

                case STATUS_PROCESSING:
                    snprintf(response, sizeof(response), "PROCESSING");
                    break;

                case STATUS_COMPLETED:
                    if (cmd->errors_count > 0) 
                    {
                        strcpy(response, "ERRORS:");
                        for (size_t i = 0; i < cmd->errors_count && i < 3; i++) 
                        {
                            strcat(response, " ");
                            strcat(response, cmd->errors[i]);
                        }
                    }   
                    else 
                    {
                        snprintf(response, sizeof(response), "COMPLETED_SUCCESS");
                    }
                    break;

                case STATUS_FAILED:
                    snprintf(response, sizeof(response), "FAILED");
                    break;

                default:
                    snprintf(response, sizeof(response), "UNKNOWN");
            }
        }
        
        printf("GUID: %s -> Status: %s\n", guid_buffer, response);
    }
    
    mq_close(status_mq);
    return NULL;
}

unsigned long guid_hash(char const *guid_buffer)
{
    unsigned long hash_code = 0;
    int i;
    for (i = 0; guid_buffer[i]; ++i)
    {
        hash_code = hash_code * 31 + guid_buffer[i];
    }

    return hash_code;
}

int guid_hash_table_init(guid_table *table)
{
    table->capacity = 16;
    
    table->buckets = (Hash_entry **)calloc(table->capacity, sizeof(Hash_entry *));
    if (table->buckets == NULL)
    {
        perror("table->buckets memory allocation");
        return -1;
    }
    
    table->size = 0;
    
    // Инициализируем мьютекс
    if (pthread_mutex_init(&table->mutex, NULL) != 0) {
        free(table->buckets);
        perror("pthread_mutex_init");
        return -1;
    }
    
    return 0;
}

int guid_table_put(guid_table *table, Validation_command *cmd)
{
    pthread_mutex_lock(&table->mutex);
    unsigned long hash_code = guid_hash(cmd->guid_v1);
    size_t index = hash_code % table->capacity;

    Hash_entry *iter = table->buckets[index];
    while (iter != NULL)
    {
        if (strcmp(iter->cmd->guid_v1, cmd->guid_v1) == 0)
        {
            //TODO: maybe need free(cmd)
            pthread_mutex_unlock(&table->mutex);
            return 1;
        }
        iter = iter->next;
    }

    Hash_entry *new_entry = (Hash_entry *)malloc(sizeof(Hash_entry));
    if (new_entry == NULL)
    {
        pthread_mutex_unlock(&table->mutex);
        perror("new_entry allocation");
        return -1;
    }

    new_entry->cmd = cmd;
    new_entry->next = table->buckets[index];
    table->buckets[index] = new_entry;
    table->size++;
    pthread_mutex_unlock(&table->mutex);
    return 0;
}

Validation_command* guid_table_get(guid_table *table, const char *guid)
{
    pthread_mutex_lock(&table->mutex);
    
    unsigned long hash_code = guid_hash(guid);
    size_t index = hash_code % table->capacity;

    Hash_entry *iter = table->buckets[index];
    while (iter != NULL)
    {
        if (strcmp(iter->cmd->guid_v1, guid) == 0)
        {
            pthread_mutex_unlock(&table->mutex);
            return iter->cmd;
        }
        iter = iter->next;
    }
    
    pthread_mutex_unlock(&table->mutex);
    return NULL;
}

int main()
{
    mqd_t mq;
    char const *mq_name = "/my_mq";
    char message_buffer[512];
    unsigned int priority;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 512;
    attr.mq_curmsgs = 0;

    mq = mq_open("/my_mq", O_CREAT | O_RDONLY, 0644, &attr);
        if (mq == (mqd_t) - 1)
            {
                perror("mq_server_open");
                return 1;
            }

    pthread_mutex_t queue_mutex;

    init_validation_queue(validation_queue);
    guid_hash_table_init(&table);
    pthread_t worker_thread;
    pthread_t status_returner_thread;
    pthread_create(&worker_thread, NULL, validation_worker_thread, NULL);
    pthread_create(&status_returner_thread, NULL, send_status_to_client, NULL);


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

        generate_simple_guide_v1(cmd_for_validation->guid_v1);

        cmd_for_validation->errors = NULL;
        cmd_for_validation->errors_count = 0;

        enQueue(&validation_queue, cmd_for_validation);
        guid_table_put(&table, cmd_for_validation);

    }


    return 0;
}