#include "console_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Läser en rad från stdin med fgets, tar bort newline */
bool console_read_input(char *buffer, uint32_t buffer_length) {
    uint32_t len;
    
    /* Validerar input-parametrar, kontrollerar alltid pekare innan användning */
    if (!buffer || buffer_length == 0) return false;

    /* Läser rad från stdin med fgets, läser max buffer_length-1 tecken */
    if (!fgets(buffer, (int)buffer_length, stdin)) return false;

    /* Beräknar längden på inläst sträng */
    len = (uint32_t)strlen(buffer);

    /* Tar bort newline-tecken om det finns */
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    return true;
}

