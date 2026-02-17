#include "thread_pool.h"

#include <string.h>

// TODO: To complete Thread_Pool
// Add a Queue so if all threads are in use then the new task is placed in Queue

void* thread_worker(Thread_Item* thread_item)
{
    while (1)
    {
        if (thread_item == NULL || thread_item->task == NULL)
            continue;

        thread_item->result = thread_item->task(thread_item->arg);
        /* thread_item->task((void*)thread_item->arg); */
        thread_item->task = NULL;
        thread_item->arg = NULL;
        thread_item->in_use = false;

        // return an enum and check if task is done, if done then return
        return thread_item->result;
    }

    return NULL;
}

// TODO: Change name and more
void *callback(void *t)
{
    thread_worker((Thread_Item*)t);
    return NULL;
}

void Thread_Pool_Create(Thread_Pool *pool)
{
    int i;
    for (i = 0; i < THREAD_POOL_SIZE; i++)
    {
        Thread_Item *item = &pool->threads[i];
        item->arg = NULL;
        item->task = NULL;
        item->in_use = false;
        item->result = NULL;
        
        pthread_create(&item->thread, NULL, callback, item);
    }

}

void Thread_Pool_Task_Add(Thread_Pool *pool, Handle_Thread_Function func, void *arg)
{
    int i;
    for (i = 0; i < THREAD_POOL_SIZE; i++)
    {
        Thread_Item *item = &pool->threads[i];
        if (!item->in_use)
        {
            item->task = func;
            item->arg = arg;
            item->in_use = true;
            break;
        }
    }
}

void Thread_Pool_Destroy(Thread_Pool *pool)
{
    if (pool == NULL)
        return;
    
    int i;
    for (i = 0; i < THREAD_POOL_SIZE; i++)
    {
        Thread_Item *item = &pool->threads[i];

        if (item->in_use)
            pthread_join(item->thread, NULL);
        else
            pthread_cancel(item->thread);


        memset(item, 0, sizeof(Thread_Item));
    }

    printf("Destroyed\n");

    //remove any malloced resources    
    
}

