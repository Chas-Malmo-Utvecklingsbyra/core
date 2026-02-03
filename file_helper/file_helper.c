#include "file_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include "../string/strdup.h"

#define MAX_PATH_LENGTH 4096

bool File_Helper_Dir_Exists(const char *path)
{
    if (!path)
        return false;
    
    struct stat info;
    if (stat(path, &info) != 0)
        return false;
    return S_ISDIR(info.st_mode);
}

bool File_Helper_File_Exists(const char *path)
{
    if (!path)
        return false;
    
    FILE *file = fopen(path, "r");
    if (!file)
        return false;

    fclose(file);
    return true;
}

File_Helper_Result File_Helper_Create_Dir(const char *path)
{
    if (!path)
        return FILE_HELPER_RESULT_FAILURE;

    if (File_Helper_Dir_Exists(path))
        return FILE_HELPER_RESULT_ALREADY_EXISTS;

    // 0755 = rwxr-xr-x permissions
    if (mkdir(path, 0755) != 0)
        return FILE_HELPER_RESULT_FAILURE;

    return FILE_HELPER_RESULT_SUCCESS;
}

File_Helper_Result File_Helper_Delete_Dir(const char *path)
{
    if (!path)
        return FILE_HELPER_RESULT_FAILURE;
        
    if (File_Helper_Dir_Exists(path) == false)
        return FILE_HELPER_RESULT_NOT_FOUND;

    int res = remove(path);

    if (res != 0)
        return FILE_HELPER_RESULT_FAILURE;

    return FILE_HELPER_RESULT_SUCCESS;
}

File_Helper_Result File_Helper_Create(const char *path, const char *filename)
{
    if (!path || !filename)
        return FILE_HELPER_RESULT_FAILURE;

    if (!File_Helper_Dir_Exists(path))
        if (File_Helper_Create_Dir(path) != FILE_HELPER_RESULT_SUCCESS)
            return FILE_HELPER_RESULT_FAILURE;

    char full_path[MAX_PATH_LENGTH] = {0};
    int result = snprintf(full_path, MAX_PATH_LENGTH, "%s/%s", path, filename);
    if (result < 0 || result >= MAX_PATH_LENGTH)
        return FILE_HELPER_RESULT_FAILURE;

    FILE *file = fopen(full_path, "w");
    if (!file)
        return FILE_HELPER_RESULT_FAILURE;

    fclose(file);
    return FILE_HELPER_RESULT_SUCCESS;
}

File_Helper_Result File_Helper_Write(const char *path, const char *filename, const char *data, size_t data_size, File_Helper_Mode mode, bool overwrite)
{
    if (!path || !filename || !data || data_size == 0 || mode == FILE_HELPER_MODE_READ)
        return FILE_HELPER_RESULT_FAILURE;

    if (!File_Helper_Dir_Exists(path))
        if(File_Helper_Create_Dir(path) != FILE_HELPER_RESULT_SUCCESS)
            return FILE_HELPER_RESULT_FAILURE;

    char full_path[MAX_PATH_LENGTH] = {0};

    int result = snprintf(full_path, MAX_PATH_LENGTH, "%s/%s", path, filename);
    if (result < 0 || result >= MAX_PATH_LENGTH)
        return FILE_HELPER_RESULT_FAILURE;

    if (!overwrite && File_Helper_File_Exists(full_path) && mode == FILE_HELPER_MODE_WRITE)
        return FILE_HELPER_RESULT_ALREADY_EXISTS;

    FILE *file;
    switch (mode)
    {
    case FILE_HELPER_MODE_WRITE:
        file = fopen(full_path, "w");
        break;
    case FILE_HELPER_MODE_APPEND:
        file = fopen(full_path, "a");
        break;

    default:
        file = NULL;
        break;
    }

    if (!file)
        return FILE_HELPER_RESULT_FAILURE;

    size_t written = fwrite(data, 1, data_size, file);
    if (written != data_size)
    {
        fclose(file);
        return FILE_HELPER_RESULT_FAILURE;
    }
    fclose(file);
    return FILE_HELPER_RESULT_SUCCESS;
}

File_Helper_Result File_Helper_Read(const char *path, const char *filename, char **out_data, size_t *out_data_size)
{
    if (!path || !filename || !out_data || !out_data_size)
        return FILE_HELPER_RESULT_FAILURE;
    
    if (!File_Helper_Dir_Exists(path))
        return FILE_HELPER_RESULT_NOT_FOUND;
    
    char full_path[MAX_PATH_LENGTH] = {0};
    int res = snprintf(full_path, MAX_PATH_LENGTH, "%s/%s", path, filename);
    if (res < 0 || res >= MAX_PATH_LENGTH)
        return FILE_HELPER_RESULT_FAILURE;
    
    FILE *file = fopen(full_path, "r");
    if (!file)
        return FILE_HELPER_RESULT_FAILURE;
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size < 0)
    {
        fclose(file);
        return FILE_HELPER_RESULT_FAILURE;
    }
    
    size_t file_size_cast = (size_t)file_size;
    char *buffer = (char *)malloc(file_size_cast + 1);
    if (!buffer)
    {
        fclose(file);
        return FILE_HELPER_RESULT_ALLOCATION_FAILURE;
    }
    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != (size_t)file_size)
    {
        free(buffer);
        fclose(file);
        return FILE_HELPER_RESULT_FAILURE;
    }

    fclose(file);
    buffer[read_size] = '\0';
    *out_data = buffer;
    *out_data_size = read_size;

    return FILE_HELPER_RESULT_SUCCESS;
}

File_Helper_Result File_Helper_Delete_File(const char *path, const char *filename)
{
    if (path == NULL || filename == NULL)
        return FILE_HELPER_RESULT_FAILURE;

    if (!File_Helper_Dir_Exists(path))
        return FILE_HELPER_RESULT_NOT_FOUND;

    char full_path[MAX_PATH_LENGTH] = {0};
    int result = snprintf(full_path, MAX_PATH_LENGTH, "%s/%s", path, filename);

    if (result < 0 || result >= MAX_PATH_LENGTH || !File_Helper_File_Exists(full_path))
        return FILE_HELPER_RESULT_FAILURE;

    if (remove(full_path) != 0)
        return FILE_HELPER_RESULT_FAILURE;

    return FILE_HELPER_RESULT_SUCCESS;
}

File_Helper_Result File_Helper_Get_Most_Recent_File(const char *path, char **out_filepath)
{
    if (!path || !out_filepath)
        return FILE_HELPER_RESULT_FAILURE;

    if (!File_Helper_Dir_Exists(path))
        return FILE_HELPER_RESULT_NOT_FOUND;

    DIR *dir = opendir(path);
    if (!dir)
        return FILE_HELPER_RESULT_FAILURE;

    struct dirent *entry;
    struct stat file_stat;
    time_t most_recent_time = 0;
    char most_recent_path[MAX_PATH_LENGTH] = {0};
    bool found = false;

    while ((entry = readdir(dir)) != NULL)
    {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[MAX_PATH_LENGTH] = {0};
        int result = snprintf(full_path, MAX_PATH_LENGTH, "%s/%s", path, entry->d_name);
        if (result < 0 || result >= MAX_PATH_LENGTH)
            continue;

        if (stat(full_path, &file_stat) != 0)
            continue;

        // Only consider regular files
        if (!S_ISREG(file_stat.st_mode))
            continue;

        if (!found || file_stat.st_mtime > most_recent_time)
        {
            most_recent_time = file_stat.st_mtime;
            strncpy(most_recent_path, full_path, MAX_PATH_LENGTH - 1);
            most_recent_path[MAX_PATH_LENGTH - 1] = '\0';
            found = true;
        }
    }

    closedir(dir);

    if (!found)
        return FILE_HELPER_RESULT_NOT_FOUND;

    *out_filepath = strdup(most_recent_path);
    if (!*out_filepath)
        return FILE_HELPER_RESULT_ALLOCATION_FAILURE;

    return FILE_HELPER_RESULT_SUCCESS;
}

bool File_Helper_Is_File_Empty(const char *path, const char *filename)
{
    if (!path || !filename)
        return true;
        
    char full_path[MAX_PATH_LENGTH] = {0};
    int result = snprintf(full_path, MAX_PATH_LENGTH, "%s/%s", path, filename);
    if (result < 0 || result >= MAX_PATH_LENGTH)
        return true;
        
    if(!File_Helper_File_Exists(full_path))
        return true;

    struct stat file_stat;
    if (stat(full_path, &file_stat) != 0)
        return true;

    return file_stat.st_size == 0;
}