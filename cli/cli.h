#ifndef CLI_H
#define CLI_H

#ifndef CLI_MAX_OPTIONS
    #define CLI_MAX_OPTIONS 10
#endif

#include <stdbool.h>

typedef void(*Argument_Callback)(void);

typedef enum
{
    Argument_Option_Integer,
    Argument_Option_String,
    Argument_Option_None
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
    full_arg: The long argument for example: 'help', which will be turned into --help.
    short_cut: The shortcut argument for example: 'h', which will be turned into -h.
    option: Should it be parsed as a string or an integer? (Argument_Option_String || Argument_Option_Integer)
    out_data: Gives the data that was passed in as argument

    returns: true on success and false on fail
*/
bool CLI_Argument_Add(CLI *cli, const char* full_arg, const char* short_cut, Argument_Option option, void* out_data);

/*
    cli: The CLI instance
    full_arg: The long argument for example: 'help', which will be turned into --help.
    short_cut: The shortcut argument for example: 'h', which will be turned into -h.
    callback: Takes in a callback function that executes when the parameter is entered on main execution.

    returns: true on success and false on fail
*/
bool CLI_Argument_Add_Callback(CLI *cli, const char* full_arg, const char* short_cut, Argument_Callback callback);

bool CLI_Parse(CLI *cli, int argc, char **argv);

#endif