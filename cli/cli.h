#ifndef CLI_H
#define CLI_H

#ifndef CLI_MAX_OPTIONS
    #define CLI_MAX_OPTIONS 10
#endif

#include <stdbool.h>

typedef void(*Argument_Callback)(void);

typedef enum
{
    Argument_Option_Has_Argument,
    Argument_Option_Has_No_Argument    
} Argument_Option;

typedef struct
{
    const char* full_arg;
    const char* short_cut;
} Argument_String;

typedef struct 
{
    Argument_String string;
    Argument_Callback callback;
    Argument_Option option;
    bool is_occupied;
    void* data;
} Option_Info;


typedef struct
{
    Option_Info infos[CLI_MAX_OPTIONS];
} CLI;

/*
    cli: The CLI instance
    argument: The long argument for example: 'help', which will be turned into --help.
    shortcut_argument: The shortcut argument for example: 'h', which will be turned into -h.
    callback: Callback function, which is called when the argument is used
*/
int CLI_Argument_Add(CLI *cli, const char* full_arg, const char* short_cut, Argument_Option option, void* out_data);
int CLI_Argument_Non_Add(CLI *cli, const char* full_arg, const char* short_cut, Argument_Callback callback);

bool CLI_Parse(CLI *cli, int argc, char **argv);

#endif