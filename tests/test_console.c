#include <stdio.h>
#include <stdbool.h>
#include "../console/console_utils.h"

int main(void)
{
    char buffer[16];

    /* NULL buffer should fail */
    if (console_read_input(NULL, sizeof(buffer)) != false)
        return 1;

    /* Zero length should fail */
    if (console_read_input(buffer, 0) != false)
        return 1;

    printf("console basic test passed\n");
    return 0;
}
