#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

typedef struct 
{
    int max_people;
    int women_inside;
    int men_inside;
    
    sem_t mutex;
    sem_t women_sem;
    sem_t men_sem;
    sem_t capacity_sem;
    
    int indicator;          // Индикатор состояния: 0-никого, 1-только женщины, 2-только мужчины
} bathroom_t;

void bathroom_init(bathroom_t *bathroom, int max_people) 
{
    bathroom->max_people = max_people;
    bathroom->women_inside = 0;
    bathroom->men_inside = 0;
    bathroom->indicator = 0;
    
    sem_init(&bathroom->mutex, 0, 1);
    sem_init(&bathroom->women_sem, 0, 1);
    sem_init(&bathroom->men_sem, 0, 1);
    sem_init(&bathroom->capacity_sem, 0, max_people);
}

void bathroom_destroy(bathroom_t *bathroom) 
{
    sem_destroy(&bathroom->mutex);
    sem_destroy(&bathroom->women_sem);
    sem_destroy(&bathroom->men_sem);
    sem_destroy(&bathroom->capacity_sem);
}

void print_indicator(bathroom_t *bathroom) 
{
    if (bathroom->women_inside > 0) 
    {
        bathroom->indicator = 1;
    } 
    else if (bathroom->men_inside > 0) 
    {
        bathroom->indicator = 2;
    } 
    else 
    {
        bathroom->indicator = 0;
    }
    
    const char *states[] = {"Пусто", "Только женщины", "Только мужчины"};
    printf("Индикатор: %s | Женщин: %d | Мужчин: %d\n", 
           states[bathroom->indicator], bathroom->women_inside, bathroom->men_inside);
}

void woman_wants_to_enter(bathroom_t *bathroom, int id) 
{
    printf("Женщина %d хочет войти в ванную\n", id);
    
    sem_wait(&bathroom->women_sem);
    
    sem_wait(&bathroom->capacity_sem);
    
    sem_wait(&bathroom->mutex);
    bathroom->women_inside++;
    
    if (bathroom->women_inside == 1) 
    {
        sem_wait(&bathroom->men_sem);
    }
    
    sem_post(&bathroom->mutex);
    
    sem_wait(&bathroom->mutex);
    printf("Женщина %d вошла в ванную. ", id);
    print_indicator(bathroom);
    sem_post(&bathroom->mutex);
    
    sem_post(&bathroom->women_sem);
}

void man_wants_to_enter(bathroom_t *bathroom, int id)
{
    printf("Мужчина %d хочет войти в ванную\n", id);
    
    sem_wait(&bathroom->men_sem);
    
    sem_wait(&bathroom->capacity_sem);
    
    sem_wait(&bathroom->mutex);
    bathroom->men_inside++;
    
    if (bathroom->men_inside == 1) 
    {
        sem_wait(&bathroom->women_sem);
    }
    
    sem_post(&bathroom->mutex);
    
    sem_wait(&bathroom->mutex);
    printf("Мужчина %d вошел в ванную. ", id);
    print_indicator(bathroom);
    sem_post(&bathroom->mutex);
    
    sem_post(&bathroom->men_sem);
}

void woman_leaves(bathroom_t *bathroom, int id) 
{
    sem_wait(&bathroom->mutex);
    bathroom->women_inside--;
    
    printf("Женщина %d вышла из ванной. ", id);
    print_indicator(bathroom);
    
    if (bathroom->women_inside == 0) 
    {
        sem_post(&bathroom->men_sem);
    }
    
    sem_post(&bathroom->mutex);
    
    sem_post(&bathroom->capacity_sem);
}

void man_leaves(bathroom_t *bathroom, int id) 
{
    sem_wait(&bathroom->mutex);
    bathroom->men_inside--;
    
    printf("Мужчина %d вышел из ванной. ", id);
    print_indicator(bathroom);
    
    if (bathroom->men_inside == 0) 
    {
        sem_post(&bathroom->women_sem);
    }
    
    sem_post(&bathroom->mutex);
    
    sem_post(&bathroom->capacity_sem);
}

typedef struct {
    bathroom_t *bathroom;
    int id;
    int gender;
} thread_args_t;

void* person_thread(void *arg) 
{
    thread_args_t *args = (thread_args_t*)arg;
    
    sleep(rand() % 3);
    
    if (args->gender == 0) 
    {    
        woman_wants_to_enter(args->bathroom, args->id);
        
        sleep(rand() % 5 + 1);
        
        woman_leaves(args->bathroom, args->id);
    } 
    else 
    {
        man_wants_to_enter(args->bathroom, args->id);
        
        sleep(rand() % 5 + 1);
        
        man_leaves(args->bathroom, args->id);
    }
    
    free(args);
    return NULL;
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        printf("Использование: %s <N> <кол-во_людей>\n", argv[0]);
        printf("  N - максимальное число человек в ванной комнате\n");
        printf("  кол-во_людей - общее количество людей для симуляции\n");
        return 1;
    }
    
    int max_people = atoi(argv[1]);
    int total_people = atoi(argv[2]);
    
    if (max_people <= 0 || total_people <= 0) 
    {
        printf("Ошибка: параметры должны быть положительными числами\n");
        return 1;
    }
    
    srand(time(NULL));
    
    bathroom_t bathroom;
    bathroom_init(&bathroom, max_people);
    
    printf("=== Симуляция ванной комнаты ===\n");
    printf("Максимальная вместимость: %d\n", max_people);
    printf("Всего людей для симуляции: %d\n\n", total_people);
    
    printf("Начальное состояние. ");
    print_indicator(&bathroom);
    printf("\n");
    
    pthread_t threads[total_people];
    
    for (int i = 0; i < total_people; i++) 
    {
        thread_args_t *args = malloc(sizeof(thread_args_t));
        if (!args) 
        {
            perror("Ошибка выделения памяти");
            continue;
        }
        
        args->bathroom = &bathroom;
        args->id = i + 1;
        args->gender = rand() % 2;
        
        if (pthread_create(&threads[i], NULL, person_thread, args) != 0) 
        {
            perror("Ошибка создания потока");
            free(args);
        }
    }
    
    for (int i = 0; i < total_people; i++) 
    {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n=== Симуляция завершена ===\n");
    
    bathroom_destroy(&bathroom);
    
    return 0;
}