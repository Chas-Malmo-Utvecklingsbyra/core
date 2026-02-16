#include "thread_pool.h"

#include <string.h>

void* thread_worker(Thread_Item* thread_item)
{
    while (1)
    {
        
        if (thread_item == NULL || thread_item->task == NULL)
            continue;
        
        if(thread_item->in_use)
            break;

        thread_item->result = thread_item->task(thread_item->arg);
        thread_item->task((void*)thread_item->arg);
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
    printf("Created1\n");
    int i;
    for (i = 0; i < THREAD_POOL_SIZE; i++)
    {
        Thread_Item *item = &pool->threads[i];
        item->arg = NULL;
        item->task = NULL;
        item->in_use = true;
        item->result = NULL;
        
        pthread_create(&item->thread, NULL, callback, item->arg);
        printf("Created2\n");
    }

}

void Thread_Pool_Task_Add(Thread_Pool *pool, Handle_Thread_Function func, void *arg)
{
    int i;
    for (i = 0; i < THREAD_POOL_SIZE; i++)
    {
        Thread_Item *item = &pool->threads[i];
        if (!item->in_use)
            continue;

        item->task = func;
        item->arg = arg;
        item->in_use = true;
        
        break;
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
        memset(item, 0, sizeof(Thread_Item));
    }

    printf("Destroyed\n");

    
    //remove any malloced resources    
    
}

