#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int CLI_Argument_Add(CLI *cli, const char* full_arg, const char* short_cut, Argument_Option option, void* out_data)
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
        return -1;
    }

    return 0;
}

int CLI_Argument_Non_Add(CLI *cli, const char* full_arg, const char* short_cut, Argument_Callback callback)
{
    bool found_place = false;

    for (size_t i = 0; i < CLI_MAX_OPTIONS; i++)
    {
        if (!cli->infos[i].is_occupied)
        {
            cli->infos[i].is_occupied = true;
            cli->infos[i].string.full_arg = full_arg;
            cli->infos[i].string.short_cut = short_cut;
            cli->infos[i].option = Argument_Option_Has_No_Argument;
            cli->infos[i].data = NULL;
            cli->infos[i].callback = callback;
            found_place = true;
            break;
        }
    }

    if (!found_place)
    {
        // Could not find a spot to add Argument.
        return -1;
    }

    return 0;
}

bool CLI_Argument_String_CMP(Argument_String* string, const char* argument)
{
    bool full_arg_success = (strcmp(string->full_arg, argument) == 0);
    bool short_cut_success = (strcmp(string->short_cut, argument) == 0);

    return full_arg_success || short_cut_success;
}


// TODO: Add checks so you cant use multiple of same arguments
bool CLI_Parse(CLI *cli, int argc, char **argv)
{
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
                if (cli->infos[i].option == Argument_Option_Has_Argument)
                {
                    ptr = argv[++index];

                    if (ptr == NULL)
                        return false;

                    int converted_int = atoi(ptr);
                    
                    printf("%s ptr to parse in CLI_Parse \n", ptr);

                    if (converted_int == 0)
                        return false;
                    
                    int *int_ptr = (int*)cli->infos[i].data;
                    *int_ptr = converted_int;
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