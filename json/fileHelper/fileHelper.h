#ifndef FILEHELPER_H
#define FILEHELPER_H

#include "../cJSON/cJSON.h"
#include <stdbool.h>

cJSON* json_read_from_file(const char* fileName);
int json_write_to_file(const char* fileName, cJSON* jsonObject);

// TODO: SS - Move these out of here.
bool file_exists(const char* fileName);
bool file_delete(const char *fileName);

#endif