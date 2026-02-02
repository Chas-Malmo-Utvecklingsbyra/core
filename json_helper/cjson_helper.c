#include "cjson_helper.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* TODO: LS add comments and test this, and check size? */
cJSON *Cjson_Helper_Read_From_File(const char *path, const char *filename)
{
    if (!path && !filename)
        return NULL;

    char *file_data = NULL;
    size_t file_data_size = 0;

    File_Helper_Result res = File_Helper_Read(path, filename, &file_data, &file_data_size);

    if (res != FILE_HELPER_RESULT_SUCCESS || file_data == NULL)
    {
        return NULL;
    }

    cJSON* root = cJSON_Parse(file_data);
    if (root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "JSON parse error before: %s\n", error_ptr);
        }
    }

    free(file_data);
    return root;
}

CJSON_Helper_Result CJSON_Helper_Write_To_File(const char *path, const char *filename, cJSON *json_data)
{
    if (!path || !filename || !json_data)
        return CJSON_HELPER_RESULT_FAILURE;

    char* json_string = cJSON_Print(json_data);
    size_t json_string_size = strlen(json_string);

    File_Helper_Result res = File_Helper_Write(path, filename, json_string, json_string_size, FILE_HELPER_MODE_WRITE, true);
    if (res != FILE_HELPER_RESULT_SUCCESS)
    {
        return CJSON_HELPER_RESULT_FAILURE;
    }

    free(json_string);
    return CJSON_HELPER_RESULT_SUCCESS;
}