#include "strdup.h"
#include <stdlib.h>
#include <string.h>

char* strdup(const char* str_to_dupe)
{
    if (str_to_dupe == NULL)
        return NULL;
    
    size_t size = strlen(str_to_dupe) + 1;
    char* new_duped = (char*)malloc(size);

    if (new_duped == NULL)
        return NULL;

    memcpy(new_duped, str_to_dupe, size);

    return new_duped;
}