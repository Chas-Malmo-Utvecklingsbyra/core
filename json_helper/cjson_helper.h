#ifndef CJSON_HELPER_H
#define CJSON_HELPER_H

#include "../json/cJSON/cJSON.h"
#include "file_helper/file_helper.h"

typedef enum CJSON_Helper_Result
{
    CJSON_HELPER_RESULT_SUCCESS,
    CJSON_HELPER_RESULT_FAILURE
} CJSON_Helper_Result;

cJSON *Cjson_Helper_Read_From_File(const char *path, const char *filename);

CJSON_Helper_Result CJSON_Helper_Write_To_File(const char *path, const char *filename, cJSON *json_data);

#endif