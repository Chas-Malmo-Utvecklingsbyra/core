#include <stdio.h>
#include "../Datatypes/LinkedList.h"

int main(void)
{
    Linked_List list;
    int value = 5;

    /* Init test */
    Linked_List_Init(&list);
    if (list.head != NULL || list.tail != NULL)
        return 1;

    /* Add test */
    Linked_List_Add_Item(&list, &value);
    if (list.head == NULL || list.tail == NULL)
        return 1;

    /* Remove test */
    Linked_List_Remove_Item(&list, &value, NULL);
    if (list.head != NULL || list.tail != NULL)
        return 1;

    printf("All tests passed\n");
    return 0;
}
