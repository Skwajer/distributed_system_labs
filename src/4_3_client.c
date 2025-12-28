// client.c
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <time.h>

#define SERVER_QUEUE "/my_mq"
#define STATUS_QUEUE "/validation_status"

int main() 
{
    mqd_t mq_for_send_guid_on_server;
    char const *mq_for_send_guid_on_server_name = "/validation_status";
    printf("=== PHONE NUMBER VALIDATION CLIENT ===\n\n");
    
    const char* phone_number = "+02842";
    int priority = 5;
    
    printf("Test data:\n");
    printf("  Phone number: %s\n", phone_number);
    printf("  Priority: %d\n\n", priority);
    
    //generate unique name for the queue receiving guid for status check
    char guid_queue_name[64];
    snprintf(guid_queue_name, sizeof(guid_queue_name), 
             "/client_%d_%ld", getpid(), time(NULL));
    
    printf("1. Creating GUID queue: %s\n", guid_queue_name);
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 128;
    attr.mq_curmsgs = 0;
    
    mqd_t guid_mq = mq_open(guid_queue_name, 
                           O_CREAT | O_RDONLY, 
                           0644, &attr);
    
    if (guid_mq == (mqd_t)-1) 
    {
        perror("❌ Failed to create GUID receive queue");
        return 1;
    }
    printf("   ✅ Queue created\n");
    
    char server_message[256];
    snprintf(server_message, sizeof(server_message), 
             "%s|%s", guid_queue_name, phone_number);
    
    printf("\n2. Sending to server queue '%s':\n", SERVER_QUEUE);
    printf("   Message: %s\n", server_message);
    
    mqd_t server_mq = mq_open(SERVER_QUEUE, O_WRONLY);
    if (server_mq == (mqd_t)-1) 
    {
        perror("❌ Failed to open server queue");
        mq_close(guid_mq);
        mq_unlink(guid_queue_name);
        return 1;
    }
    
    if (mq_send(server_mq, server_message, strlen(server_message) + 1, priority) == -1) 
    {
        perror("❌ Failed to send to server");
        mq_close(server_mq);
        mq_close(guid_mq);
        mq_unlink(guid_queue_name);
        return 1;
    }
    
    printf("   ✅ Message sent successfully\n");
    sleep(2);
    
    printf("\n3. Waiting for GUID from server...\n");
    
    char response[128];
    ssize_t bytes = mq_receive(guid_mq, response, sizeof(response), NULL);
    
    if (bytes == -1) {
        perror("❌ Failed to receive GUID");
        mq_close(server_mq);
        mq_close(guid_mq);
        mq_unlink(guid_queue_name);
        return 1;
    }
    
    response[bytes] = '\0';
    printf("   ✅ Response received\n");
    
    if (strncmp(response, "GUID:", 5) == 0) 
    {
        char* guid = response + 5;
        printf("\n═══════════════════════════════════════\n");
        printf("✅ SUCCESS! Validation request accepted\n");
        printf("═══════════════════════════════════════\n\n");
        printf("Your GUID: %s\n\n", guid);


        //send the guid to the server for validating status check
        mq_for_send_guid_on_server = mq_open(mq_for_send_guid_on_server_name, O_WRONLY);
        if (mq_for_send_guid_on_server == (mqd_t)-1) 
        {
        perror("❌ Failed to open GUID send queue");
        return 1;
        }

        if (mq_send(mq_for_send_guid_on_server, guid, strlen(guid) + 1, 0) == -1)
        {
            perror("❌ Failed to send to server");
        mq_close(server_mq);
        mq_close(guid_mq);
        mq_unlink(guid_queue_name);
        }
    } 
    else 
    {
        printf("❌ Unexpected response: %s\n", response);
    }
    
    printf("\n7. Cleaning up...\n");
    sleep(1);
    
    mq_close(server_mq);
    mq_close(guid_mq);
    
    if (mq_unlink(guid_queue_name) == -1) {
        perror("Warning: Failed to unlink queue");
    } else {
        printf("   ✅ Queue removed\n");
    }
    
    printf("\n=== CLIENT FINISHED ===\n");
    return 0;
}