#ifndef Linked_List_H
#define Linked_List_H

#define LL_SUCCESS 0
#define LL_FAILURE -1

typedef struct Linked_List Linked_List;

typedef struct Linked_List_Node Linked_List_Node;

typedef void (*dispose_object_call_back)(void* _obj);

struct Linked_List 
{
    Linked_List_Node* head;
    Linked_List_Node* tail;
};

struct Linked_List_Node
{
    void* object;
    Linked_List_Node* next;
    Linked_List_Node* prev;
};

int Linked_List_Init(Linked_List* linked_list);

int Linked_List_Add_Item(Linked_List* linked_list, void* _object);

int Linked_List_Remove_Item(Linked_List* linked_list, void* object, dispose_object_call_back call_back);

void Linked_List_Dispose(Linked_List* linked_list, dispose_object_call_back call_back);

#endif