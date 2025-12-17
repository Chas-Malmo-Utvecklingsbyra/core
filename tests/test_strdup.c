#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../string/strdup.h"

int main(void)
{
    const char* text = "hello";
    char* copy = strdup(text);

   /* Test normal case */ 
    if (copy == NULL)
        return 1;

        if (strcmp(copy, text)!= 0)
        return 1;

    free(copy);
    
    /* Test NULL input */
    if (strdup(NULL) != NULL)
        return 1;

    printf("strdup test passed\n");
    return 0;
}