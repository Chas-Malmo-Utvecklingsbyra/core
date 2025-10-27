#include <stdlib.h>
#include "LinkedList.h"

/*
 * TODO:
 * Add Max Size -> replace oldest object?
 * Add handshake / type verification
 * Simple doubly-linked list implementation.
 *
 * Notes:
 * - Linked_List_AddItem appends the provided object pointer to the tail of the list.
 * - Functions return 0 on success and -1 on failure (e.g. out-of-memory or invalid args).
 */

#define LL_SUCCESS 0
#define LL_FAILURE -1

/* Internal helper: allocate a new node and initialize it.
 * Returns pointer to node on success, NULL on allocation failure.
 */
static Linked_List_Node* Linked_List_Node_Create(void* object)
{
    Linked_List_Node* node = (Linked_List_Node*)malloc(sizeof(Linked_List_Node));
    if (!node) return NULL;
    node->object = object;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

/* Initialize the linked list: set head and tail to NULL.
 * Returns 0 on success, -1 on invalid parameter.
 */
int Linked_List_Init(Linked_List* linked_list)
{
    if (!linked_list) return LL_FAILURE;
    linked_list->head = NULL;
    linked_list->tail = NULL;
    return LL_SUCCESS;
}

/* Add an item (object pointer) to the tail of the list.
 * Returns 0 on success, -1 on failure (invalid args or allocation failure).
 */
int Linked_List_Add_Item(Linked_List* linked_list, void* object)
{
    if (!linked_list) return LL_FAILURE;

    Linked_List_Node* node = Linked_List_Node_Create(object);
    if (!node) return LL_FAILURE;

    if (linked_list->tail == NULL)
    {
        /* empty list */
        linked_list->head = node;
        linked_list->tail = node;
    } 
    else 
    {
        node->prev = linked_list->tail;
        linked_list->tail->next = node;
        linked_list->tail = node;
    }

    return LL_SUCCESS;
}

/* Remove object from list and dispose of object data if dispose function is provided */
int Linked_List_Remove_Item(Linked_List* linked_list, void* object, dispose_object_call_back call_back)
{
    if (!linked_list) return LL_FAILURE;

    Linked_List_Node* cur = linked_list->head;
    while (cur) 
    {
        if (cur->object == object) 
        {
            
            if (cur->prev) 
            {
                cur->prev->next = cur->next;
            } 
            else 
            {
                linked_list->head = cur->next;
            }

            if (cur->next) 
            {
                cur->next->prev = cur->prev;
            } 
            else 
            {
                linked_list->tail = cur->prev;
            }

            if (call_back && cur->object) 
            {
                call_back(cur->object);
            }

            free(cur);
            return LL_SUCCESS;
        }
        cur = cur->next;
    }

    return LL_FAILURE; /* not found */
}

/* Dispose list and object data if dispose function is provided. */
void Linked_List_Dispose(Linked_List* linked_list, dispose_object_call_back call_back)
{
    if (!linked_list) return;

    Linked_List_Node* cur = linked_list->head;
    while (cur) 
    {
        Linked_List_Node* next = cur->next;
        if (call_back && cur->object) 
        {
            call_back(cur->object);
        }
        free(cur);
        cur = next;
    }

    linked_list->head = NULL;
    linked_list->tail = NULL;
}