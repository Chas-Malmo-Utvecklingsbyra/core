#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../string/strdup.h"

bool CLI_Argument_Add(CLI *cli, const char* full_arg, const char* short_cut, Argument_Option option, void* out_data)
{
    bool found_place = false;

    for (size_t i = 0; i < CLI_MAX_OPTIONS; i++)
    {
        if (!cli->infos[i].is_occupied)
        {
            cli->infos[i].is_occupied = true;
            cli->infos[i].string.full_arg = full_arg;
            cli->infos[i].string.short_cut = short_cut;
            cli->infos[i].option = option;
            cli->infos[i].data = out_data;
            found_place = true;
            break;
        }
    }

    if (!found_place)
    {
        // Could not find a spot to add Argument.
        return false;
    }

    return true;
}

bool CLI_Argument_Add_Callback(CLI *cli, const char* full_arg, const char* short_cut, Argument_Callback callback)
{
    bool found_place = false;

    for (size_t i = 0; i < CLI_MAX_OPTIONS; i++)
    {
        if (!cli->infos[i].is_occupied)
        {
            cli->infos[i].is_occupied = true;
            cli->infos[i].string.full_arg = full_arg;
            cli->infos[i].string.short_cut = short_cut;
            cli->infos[i].option = Argument_Option_None;
            cli->infos[i].data = NULL;
            cli->infos[i].callback = callback;
            found_place = true;
            break;
        }
    }

    if (!found_place)
    {
        // Could not find a spot to add Argument.
        return false;
    }

    return true;
}

bool CLI_Argument_String_CMP(Argument_String* string, const char* argument)
{
    bool full_arg_success = (strcmp(string->full_arg, argument) == 0);
    bool short_cut_success = (strcmp(string->short_cut, argument) == 0);

    return full_arg_success || short_cut_success;
}


bool CLI_Parse(CLI *cli, int argc, char **argv)
{
    // TODO: Add checks so you cant use multiple of same arguments
    
    (void)argc;

    size_t index = 1;
    char* ptr = argv[index];
    while (ptr != NULL)
    {

        for (size_t i = 0; i < CLI_MAX_OPTIONS; i++)
        {
            if (!cli->infos[i].is_occupied)
                break;

            if (CLI_Argument_String_CMP(&cli->infos[i].string, ptr))
            {
                bool is_convertible = 
                    cli->infos[i].option == Argument_Option_Integer ||
                    cli->infos[i].option == Argument_Option_String;

                if (is_convertible)
                {
                    ptr = argv[++index];

                    if (ptr == NULL)
                        return false;

                    if (cli->infos[i].option == Argument_Option_Integer)
                    {
                        int converted_int = atoi(ptr);
                        
                        printf("%s ptr to parse in CLI_Parse \n", ptr);

                        if (converted_int == 0)
                            return false;
                        
                        int *int_ptr = (int*)cli->infos[i].data;
                        *int_ptr = converted_int;
                    }
                    else
                    {
                        char *str_ptr = (char*)cli->infos[i].data;

                        // TODO: 
                        // - Check how big str_ptr is so it doesnt buffer overflow
                        strcpy(str_ptr, ptr);
                    }
                }
                else
                {
                    cli->infos[i].callback();
                }
                
                break;
            }
        }

        ptr = argv[++index];
    }

    return true;
}