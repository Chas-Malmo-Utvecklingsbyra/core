#include "console_utils.h"
#include <stdio.h>

int main(void) {
    printf("Test: console_is_quit_command med 'exit'\n");
    
    bool result = console_is_quit_command("exit");
    
    if (result == true) {
        printf("Test godkänt: Returnerade true för 'exit'\n");
        return 0;
    } else {
        printf("Test misslyckades: Returnerade inte true för 'exit'\n");
        return 1;
    }
}