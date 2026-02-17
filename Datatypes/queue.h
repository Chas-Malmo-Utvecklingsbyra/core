#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>

typedef struct Queue_Item Queue_Item;
struct Queue_Item 
{
    void *data;
    Queue_Item *next;
};


typedef struct
{
    Queue_Item* first;
    Queue_Item* last;
    size_t size;
}Queue;

Queue Queue_Create(void);

void Queue_Enqueue(Queue *queue, void *item);

Queue_Item *Queue_Dequeue(Queue* queue);



#endif