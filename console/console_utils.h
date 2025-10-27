#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>
#include <stdbool.h>

typedef unsigned int uint32_t;


/* Läser en rad från stdin med fgets, tar bort newline */
bool console_read_input(char *buffer, uint32_t buffer_length); 


/* Rensar stdin-bufferten */
void console_flush_input(void);

/* Kontrollerar om användaren vill avsluta programmet */
bool console_is_quit_command(const char *input);

#endif

