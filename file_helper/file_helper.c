#include "file_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
        return FILE_HELPER_RESULT_NOT_FOUND;

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