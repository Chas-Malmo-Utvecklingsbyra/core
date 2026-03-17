#include "Unordered_List.h"

#include <stdlib.h>
#include <string.h>

void Unordered_List_Initialize(Unordered_List *list)
{
    if (list == NULL)
        return;

    list->data = NULL;
    list->elements = 0;
    list->capacity = 4;
}

void Unordered_List_Add(Unordered_List *list, void *item)
{
    if (list == NULL)
        return;

    if (item == NULL)
        return;

    if (list->data == NULL)
    {
        list->data = (void*)malloc(sizeof(void*) * list->capacity);
        if (list->data == NULL)
        {
            printf("Unordered_List_Add failed to malloc starting DATA\r\n");
            return;
        }

        memset(list->data, 0, sizeof(void*) * list->capacity);
        list->data[0] = item;
        list->elements++;
    }
    else
    {
        if (list->elements == list->capacity)
        {
            list->capacity *= 2;
            
            list->data = realloc(list->data, sizeof(void*) * list->capacity);
            if (list->data == NULL)
            {
                printf("Unordered_List_Add failed to realloc increased capacity\r\n");
                return;              
            }

            memset(&list->data[list->elements], 0, sizeof(void*) * (list->capacity - list->elements));
            list->data[list->elements] = item;
            list->elements++;
        }
        else
        { 
            list->data[list->elements] = item;
            list->elements++;
        }
    }

}


void Unordered_List_Remove(Unordered_List *list, size_t index)
{
    if (list == NULL)
        return;

    if (list->data == NULL)
        return;

    if (list->data[index] == NULL)
        return;

    list->data[index] = list->data[list->elements-1];
    list->data[list->elements-1] = NULL;
    list->elements--;
}

void Unordered_List_Print(Unordered_List *list)
{
    if (list == NULL)
    {
        printf("List is NULL\r\n");
        return;
    }

    if (list->data == NULL)
    {
        printf("List->Data is NULL\r\n");
        return;
    }

    if (list->elements == 0)
    {
        printf("List is empty\r\n");
        return;
    }

    for (size_t i = 0; i < list->elements; i++)
    {
        if (list->data[i] == NULL)
        {
            printf("List->Elements is NULL");
            return;
        }

        printf("[%ld] %p\r\n", i, list->data[i]);
    }
}

void Unordered_List_Dispose(Unordered_List *list)
{
    if (list == NULL)
        return;

    if (list->data == NULL)
        return;

    free(list->data);
}