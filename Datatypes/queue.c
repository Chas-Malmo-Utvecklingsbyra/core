#include "queue.h"

#include <stdlib.h>

Queue Queue_Create(void)
{
    return (Queue){.first = NULL, .last = NULL, .size = 0};
}

void Queue_Enqueue(Queue *queue, void *item)
{
    if (queue == NULL)
        return;
    
    Queue_Item *queue_item = (Queue_Item*)malloc(sizeof(Queue_Item));

    // Queue is empty
    if (queue->first == NULL)
    {
        queue->first = queue_item;
        queue_item->data = item;
        queue->size++;
        return;
    }

    Queue_Item *ptr = queue->first;
    while (1)
    {
        if (ptr->data == NULL)
        {
            (--ptr)->data = item;
            queue->size++;
            break;
        }

        ptr++;
    }
}

Queue_Item *Queue_Dequeue(Queue *queue)
{
    if (queue == NULL)
        return NULL;

    Queue_Item *first_place = queue->first;

    if (first_place == NULL)
        return NULL;

    Queue_Item *second_place = first_place->next;

    if (second_place == NULL)
    {
        queue->first = NULL;
        first_place->next = NULL;
        queue->size--;

        return first_place;
    }

    queue->first = second_place;
    first_place->next = NULL;
    queue->size--;

    return first_place;
}