#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fileHelper.h"

/* Creates and returns a cJSON pointer that contains
the data from the parsed JSON file */
cJSON* Read_JSON_From_File(const char* _FileName)
{
    /* Opens file in read mode and 
    ensures that it was successful */
    FILE *file = fopen(_FileName, "r");
    if (file == NULL)
    {
        return NULL;
    }

    /* Goes through the contents of the file
    to find how many characters needs memory allocated */
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* JsonString = calloc(1, length + 1);
    if (JsonString == NULL)
    {
        fclose(file);
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    /* Transfers contents of the JSON file to the
    JsonString buffer, then parses the data using cJSON */
    fread(JsonString, 1, length, file);
    fclose(file);

    cJSON* root = cJSON_Parse(JsonString);
    if (root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "JSON parse error before: %s\n", error_ptr);
        }
    }

    free(JsonString);
    return root;
}

/* Returns 0 for success and -1 for failure */
int Write_Json_To_File(const char* _FileName, cJSON* _JSON_Object)
{
    /* Opens file in write mode and uses cJSON_Print to
    create valid JSON data that is then printed into file */
    assert(_JSON_Object != NULL);
    FILE *f = fopen(_FileName, "w");
    if (f == NULL)
    {
        printf("Error opening file\n");
        return -1;
    }
    char* tempJsonStr = cJSON_Print(_JSON_Object);

    fprintf(f, "%s\n", tempJsonStr);
    fclose(f);
    free(tempJsonStr);
    return 0;
}

/* Attempts to open file in order to determine if it already exists */
int DoesFileExist(const char* _FileName)
{
    FILE *file = fopen(_FileName, "r");
    if (file)
    {
        fclose(file);
        return 1;
    } else {
        return 0;
    }
}

/* Same as Read_JSON_From_File except it returns a 
char* instead of a cJSON pointer */
char* Read_JSON_From_File_Return_Char(const char* _FileName)
{
    FILE *file = fopen(_FileName, "r");
    if (file == NULL)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* JsonString = calloc(1, length + 1);
    if (JsonString == NULL)
    {
        fclose(file);
        printf("Memory allocation failed\n");
        return NULL;
    }
    fread(JsonString, 1, length, file);
    fclose(file);
    JsonString[length] = '\0';
    return JsonString;
}

int DeleteFile(const char* _FileName)
{
    if (remove(_FileName) == 0)
    {
        return 0;
    } else {
        perror("Failed to delete file");
        return -1;
    }
}