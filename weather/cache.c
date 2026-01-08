#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "cache.h"

Cache_Result cache_save_to_file(const char* file_name, const char* data)
{
    cache_create_directory();
    
    struct stat file_stat;
    int exists = stat(file_name, &file_stat);
    if (exists == 0)
    {
        time_t current_time;
        time(&current_time);

        double difference = difftime(current_time, file_stat.st_mtime);

        printf("Current time: %s", ctime(&current_time));
        printf("File modification time: %s", ctime(&file_stat.st_mtime));
        printf("Difference is %.0f seconds\n", difference);
    }
    

    FILE* file = fopen(file_name, "w");
    if (file == NULL)
        return Cache_Result_Error;
    
    fprintf(file, data);

    fclose(file);

    return Cache_Result_OK;
}

Cache_Result cache_create_directory()
{
    const char* dir_name = "build/cache";
    struct stat dir_stat;
    if (stat(dir_name, &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode))
    {
        return Cache_Result_OK;
    }
    
    int result = mkdir(dir_name, 0755);
    if (result == 0)
    {
        printf("Successfully created directory %s\n", dir_name);
        return Cache_Result_OK;
    }
    else
    {
        printf("Failed to create directory %s\n", dir_name);
        return Cache_Result_Error;
    }
}