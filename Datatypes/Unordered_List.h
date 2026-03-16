#ifndef UNORDERED_LIST_H
#define UNORDERED_LIST_H

#include <stdio.h>

typedef struct 
{
    void** data;
    size_t elements;
    size_t capacity;
} Unordered_List;

void Unordered_List_Initialize(Unordered_List *list);

void Unordered_List_Add(Unordered_List *list, void *item);

void Unordered_List_Remove(Unordered_List *list, size_t index);

void Unordered_List_Print(Unordered_List *list);

void Unordered_List_Dispose(Unordered_List *list);

#endif