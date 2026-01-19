#include "query_parameters.h"
#include <stdlib.h>
#include <string.h>

/* TODO: LS Test */
QueryParameter_Result_t Query_Parameter_Create(QueryParameters_t *param, size_t param_count)
{
    if (!param || param_count == 0)
        return QUERY_PARAMETER_RESULT_ERROR; /* Invalid arguments */

    param->keys = (char **)malloc(sizeof(char *) * param_count);
    if (param->keys == NULL)
    {
        return QUERY_PARAMETER_RESULT_ALLOC_FAILURE; /* Memory allocation failure */
    }
    
    param->values = (char **)malloc(sizeof(char *) * param_count);
    if (param->values == NULL)
    {
        if (param->keys != NULL)
            free(param->keys);
        
        return QUERY_PARAMETER_RESULT_ALLOC_FAILURE; /* Memory allocation failure */
    }

    /* Allocate memory for each key and value */
    for (size_t i = 0; i < param_count; i++)
    {
        param->keys[i] = (char *)malloc(sizeof(char) * QUERY_PARAMETER_MAX_LENGTH);
        if (param->keys[i] == NULL) /* allocation check */
        {
            for (size_t j = 0; j < i; j++) /* Free previously allocated memory on failure */
            {
                if(param->keys[j] != NULL)
                    free(param->keys[j]);
                if (param->values[j] != NULL)
                    free(param->values[j]);
            }
            free(param->keys);
            free(param->values);
            return QUERY_PARAMETER_RESULT_ALLOC_FAILURE; /* Memory allocation failure */
        }
        
        param->values[i] = (char *)malloc(sizeof(char) * QUERY_PARAMETER_MAX_LENGTH);
        if (param->values[i] == NULL) /* allocation check */
        {
            for (size_t j = 0; j <= i; j++) /* Free previously allocated memory on failure */
            {
                if(param->keys[j] != NULL)
                    free(param->keys[j]);
                if(param->values[j] != NULL)
                    free(param->values[j]);
            }
            free(param->keys);
            free(param->values);
            return QUERY_PARAMETER_RESULT_ALLOC_FAILURE; /* Memory allocation failure */
        }
    }
    param->count = 0;
    return QUERY_PARAMETER_RESULT_OK; /* Success */
}

QueryParameter_Result_t Query_Parameter_Parse(QueryParameters_t *param, const char *path)
{
    if (!param || !path)
        return QUERY_PARAMETER_RESULT_ERROR; /* Invalid arguments */

    size_t number_of_params = 0;
    size_t key_index = 0;
    size_t value_index = 0;

    /* Find the '?' to locate query string start */
    const char *query_start = strchr(path, '?');
    if (query_start == NULL)
        return QUERY_PARAMETER_RESULT_ERROR; /* No query parameters or invalid request */

    size_t path_length = strlen(path);
    size_t i = query_start - path;
    for (; i < path_length; i++)
    {
        if (path[i] == '?')
        {
            key_index = i + 1;
            if (key_index >= path_length)
                return QUERY_PARAMETER_RESULT_MALFORMED; /* No query parameters */
        }
        else if (path[i] == '=') /* End of parameter key, start of parameter value */
        {
            if (i == key_index)
                return QUERY_PARAMETER_RESULT_MALFORMED; /* Empty key, malformed URL */
            
            size_t key_length = (i - key_index);
            if (key_length >= QUERY_PARAMETER_MAX_LENGTH)
                return QUERY_PARAMETER_RESULT_MALFORMED; /* Key too long */
            
            strncpy(param->keys[number_of_params], &path[key_index], key_length);
            param->keys[number_of_params][key_length] = '\0';

            value_index = i + 1;
            if (value_index >= path_length || path[value_index] == '&')
                return QUERY_PARAMETER_RESULT_MALFORMED; /* Key but empty value, malformed URL, or should empty value be allowed? */
        }
        else if ((path[i] == '&' || i == path_length - 1)) /* End of parameter value, start of next key */
        {
            size_t value_length = (path[i] == '&') ? (i - value_index) : (i - value_index + 1);
            if (value_length >= QUERY_PARAMETER_MAX_LENGTH)
                return QUERY_PARAMETER_RESULT_MALFORMED; /* Value too long */

            strncpy(param->values[number_of_params], &path[value_index], value_length);
            param->values[number_of_params][value_length] = '\0';

            key_index = i + 1;
            number_of_params++;
        }
    }
    param->count = number_of_params;

    return QUERY_PARAMETER_RESULT_OK;
}

size_t Query_Parameter_Get_Param_Count(const char* path)
{
    const char *query_start = strchr(path, '?');
    if (query_start == NULL)
        return 0; /* No query parameters or invalid request */

    size_t total_params = 1;
    for (size_t i = query_start - path; i < strlen(path); i++)
    {
        if (path[i] == '&')
            total_params++;
    }
    return total_params;
}

const char *Query_Parameter_Get(QueryParameters_t *param, const char *key)
{
    if (param == NULL || key == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < param->count; i++)
    {
        if (strcmp(param->keys[i], key) == 0)
        {
            return param->values[i];
        }
    }
    return NULL; /* Key not found */
}

void Query_Parameter_Dispose(QueryParameters_t *param)
{
    if (param == NULL || param->count <= 0)
    {
        return;
    }

    if (param->keys != NULL)
    {
        for (size_t i = 0; i < param->count; i++)
        {
            if (param->keys[i] == NULL) continue;
            free(param->keys[i]);
            param->keys[i] = NULL;
        }
        free(param->keys);
        param->keys = NULL;
    }

    if (param->values != NULL)
    {
        for (size_t i = 0; i < param->count; i++)
        {
            if (param->values[i] == NULL) continue;
            free(param->values[i]);
            param->values[i] = NULL;
        }
        free(param->values);
        param->values = NULL;
    }
}