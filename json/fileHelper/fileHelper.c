#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fileHelper.h"

/* Creates and returns a cJSON pointer that contains
the data from the parsed JSON file */
cJSON* Read_JSON_From_File(const char* fileName)
{
    /* Opens file in read mode and 
    ensures that it was successful */
    FILE *file = fopen(fileName, "r");
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

    /* Transfers contents of the JSON file and stores it
    in the JsonString buffer, then parses the data using cJSON */
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

