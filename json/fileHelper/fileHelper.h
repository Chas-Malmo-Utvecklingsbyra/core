#ifndef FILEHELPER_H
#define FILEHELPER_H

#include "../cJSON/cJSON.h"

cJSON* Read_JSON_From_File(const char* _FileName);

int Write_Json_To_File(const char* _FileName, cJSON* JSON_Object);

char* Read_JSON_From_File_Return(const char* _FileName);

int DoesFileExist(const char* _FileName);

int DeleteFile(_FileName);

#endif