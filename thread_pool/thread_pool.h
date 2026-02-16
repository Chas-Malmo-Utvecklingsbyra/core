#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

#define THREAD_POOL_SIZE 32


typedef void *(*Handle_Thread_Function)(void *);

typedef struct
{
    pthread_t thread;
    void *arg;
    Handle_Thread_Function task;
    bool in_use;
    bool is_running;
    void *result;
} Thread_Item;

typedef struct
{
    Thread_Item threads[THREAD_POOL_SIZE];
    //pthread_mutex_t *pool_mutex;
} Thread_Pool;

void Thread_Pool_Create(Thread_Pool *pool);
void Thread_Pool_Task_Add(Thread_Pool *pool, Handle_Thread_Function func, void* arg);
void Thread_Pool_Destroy(Thread_Pool *pool);

#endif