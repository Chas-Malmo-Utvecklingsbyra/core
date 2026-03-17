#ifndef UNORDERED_LIST_H
#define UNORDERED_LIST_H

#include <stdio.h>

/**
 * Unordered List
 *
 * -------------------------
 * Design
 * -------------------------
 * This is a dynamic array-based list that stores generic pointers (void*).
 * The list is unordered, meaning element order is not preserved when removing elements.
 * Removal is optimized by replacing the removed element with the last element.
 *
 * The list grows dynamically using malloc and realloc.
 * Initial capacity is 4, and capacity doubles when the array becomes full.
 * The list does NOT shrink to avoid frequent reallocations and performance overhead,
 * similar to the behavior of std::vector in C++.
 *
 * Automatic deallocation of stored elements is not performed to provide the ability
 * to store stack memory.
 *
 * -------------------------
 * Memory Model
 * -------------------------
 * - The internal array (data) is allocated on the heap using malloc/realloc.
 * - The list stores only pointers (void*), not the actual data.
 * - The user is responsible for managing the memory of the stored elements.
 * - New memory is zero-initialized using memset.
 * - No automatic deallocation of stored elements is performed.
 *
 *
 * -------------------------
 * Performance Considerations
 * -------------------------
 * Add:
 *   - O(1) most of the time
 *   - O(n) worst case (when resizing occurs due to realloc)
 *
 * Remove:
 *   - O(1) time
 *   - Achieved by swapping the removed element with the last element
 *
 * Access:
 *   - O(1) time
 *
 * Print:
 *   - O(n) time (iterates through all elements)
 *
 *
 * -------------------------
 * Limitations / Bad Use Cases
 * -------------------------
 * - Not suitable when element order must be preserved
 * - No type safety due to use of void*
 * - Potential memory leaks if user does not free stored elements, unless stack memory is used.
 *
 */

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