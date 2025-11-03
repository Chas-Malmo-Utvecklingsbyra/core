#ifndef FILEHELPER_H
#define FILEHELPER_H

#include "../cJSON/cJSON.h"

cJSON* json_read_from_file(const char* fileName);

int json_write_to_file(const char* fileName, cJSON* jsonObject);

char* json_read_from_file_return(const char* fileName);

int file_exists(const char* fileName);

int file_delete(fileName);

#endif