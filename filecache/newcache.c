#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "newcache.h"


bool does_file_exist(const char* file_name)
{
    if (!file_name)
    {
        return false;
    }

    struct stat file_info;
    
    return stat(file_name, &file_info) == 0;    
}


/* The program will check if the information stored in the file is fresh (modified within the last 15 minutes). 
If it isn't fresh, the program will return false, so that new data can be collected. 
If it is fresh, it reads the files content and it will return true */
bool read_data_from_file_if_fresh(const cache_struct *cache, const char* file_name, char **out_data, size_t* data_size)
{
    if(!cache || !file_name || !out_data || !data_size)
    {
        return false;
    }

    struct stat file_info;
    if(stat(file_name, &file_info) != 0)
    {
        return false;
    }

    uint32_t time_now = time(NULL);
    double time_difference = difftime(time_now, file_info.st_mtime);
    if(time_difference > cache->time_stamp)
    {
        printf("Data in file is outdated! (cached %.0lf seconds ago).\n", time_difference);
        return false;
    }

    FILE *file = fopen(file_name, "r");
    if(!file)
    {
        return false;
    }

    *data_size = file_info.st_size;
    *out_data = (char *)malloc(*data_size + 1);
    if(!*out_data)
    {
        fclose(file);
        return false;
    }

    size_t bytes_read = fread(*out_data, 1, *data_size, file);
    fclose(file);

    (*out_data)[bytes_read] = '\0';
    
    return true;
}


bool get_data_from_file(cache_struct *cache, const char *file_name, char **out_data, size_t *data_size, cache_callback fetch_callback)
{
    if(!cache || !file_name || !out_data || !data_size)
    {
        return false;
    }

    *out_data = NULL;
    *data_size = 0;    
    
    if(read_data_from_file_if_fresh(cache, file_name, out_data, data_size))
    {
        printf("Fresh file found: %s\n", file_name);
        return true;
    }

    printf("Fetching new data for file...\n");

    if(!fetch_callback)
    {
        return false;
    }

    const char *new_data = fetch_callback();
    if(!new_data)
    {
        return false;
    }

    size_t len = strlen(new_data);

    FILE *file = fopen(file_name, "w");
    if(!file)
    {
        return false;
    }

    fwrite(new_data, 1, len, file);
    fclose(file);

    *out_data = (char *)malloc(len + 1);
    if(!*out_data)
    {
        return false;
    }

    memcpy(*out_data, new_data, len + 1);
    *data_size = len;

    return true;
}
