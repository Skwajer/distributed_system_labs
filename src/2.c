#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LOGIN 7
#define MAX_USERS_COUNT 150

#define LOGOUT_ERROR 1
#define WRONG_CHOICE_NUMBER 2


typedef struct
{
    char login[MAX_LOGIN];
    unsigned int pin;
    int banned;
} user;

int logout(user *users, size_t registred_users_count);
int get_time();
int get_date();
int how_much();
int set_sanctions(char const *username);
int str_isalnum(char const *);

int main()
{
    int status;
    user all_registred_users[MAX_USERS_COUNT];
    size_t registered_users_count = 0;
    char input_command[10];
    printf("====Welcome====\n");
    status = logout(all_registred_users, registered_users_count);
    if (status == LOGOUT_ERROR)
    {
        while (status == LOGOUT_ERROR)
        {
            status = logout(all_registred_users, registered_users_count);
        }
    }
    printf("the following list of comands avail for you: \n{time}\n{date}\n{howmuch}\n{logout}\n{sanctions}\n");

}

int logout(user *users, size_t registred_users_count)
{
    int i;
    unsigned int choice;
    unsigned int input_pin = 0;
        char input_login[7];
    printf("\nyou need authorize\n1.log in\n2.sign in\ne.exit\n");
    if (scanf("%u", &choice) != 1)
    {
        exit(0);
    }
    if (choice == 1)
    {
        printf("enter your login to enter(6 symbols max): ");
        scanf("%6s", input_login);
    }

    else if (choice == 2) 
    {
        printf("enter new login to register(6 symbols max): ");
        scanf("%6s", input_login);
        if (str_isalnum(input_login) == 0)
        {
            return LOGOUT_ERROR;
        }
        for (i = 0; i < registred_users_count; ++i)
        {
            if (strcmp(users[i].login, input_login) == 0)
            {
                printf("\nsuch login already exist");
                return LOGOUT_ERROR;
            }
        }
        printf("\ngenerate new pincode(0-100000): ");
        if ((scanf("%u", &input_pin) != 1) || (input_pin > 100000))
        {
            return LOGOUT_ERROR;
        }
        
        strcpy(users[registred_users_count].login, input_login);
        users[registred_users_count].pin = input_pin;
        users[registred_users_count].banned = 0;
        printf("you have successfully logged in, %s!\n\n", users[registred_users_count].login);
        ++registred_users_count;
        return 0;
    }


    return 0;
}

int str_isalnum(char const *login)
{
    int i;
    int digit_flag = 0, alpha_flag = 0;
    size_t len = strlen(login);
    for (i= 0; i < len; ++i)
    {
        if (isalpha(login[i]))
        {
            alpha_flag = 1;
        }
        else if (isdigit(login[i])) 
        {
            digit_flag = 1;
        }
        else
        {
            return 0;
        }
    }
    return (alpha_flag && digit_flag);
}