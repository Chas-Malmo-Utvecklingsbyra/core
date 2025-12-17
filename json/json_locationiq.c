#include "json_locationiq.h"
#include <stdlib.h>
#include <stdbool.h>

char *locationiq_json_parse(const char *locationiq_api_response, int limit)
{
    cJSON *parsed_json = cJSON_Parse(locationiq_api_response);
    if (parsed_json == NULL)
    {
        return NULL;
    }

    size_t json_array_size = cJSON_GetArraySize(parsed_json);
    if (json_array_size == 0)
    {
        cJSON_Delete(parsed_json);
        return NULL;
    }
    
    cJSON* return_object = cJSON_CreateArray();
    
    size_t i = 0;
    for (; i < json_array_size; i++)
    {
        if (return_object != NULL && cJSON_GetArraySize(return_object) >= limit)
            break; /* reached the desired number of results */

        cJSON *object = cJSON_GetArrayItem(parsed_json, i);
        if (object == NULL)
            continue;
        
        char *display_name_str = NULL;
        char *latitude_str = NULL;
        char *longitude_str = NULL;
        
        cJSON *item = cJSON_GetObjectItemCaseSensitive(object, "display_name");
        if (item != NULL && item->type == cJSON_String)
        {
            display_name_str = item->valuestring;
        }

        item = cJSON_GetObjectItemCaseSensitive(object, "lat");
        if (item != NULL && item->type == cJSON_String)
        {
            latitude_str = item->valuestring;
        }

        item = cJSON_GetObjectItemCaseSensitive(object, "lon");
        if (item != NULL && item->type == cJSON_String)
        {
            longitude_str = item->valuestring;
        }
        
        if (display_name_str != NULL && latitude_str != NULL && longitude_str != NULL)
        {
            double latitude_double = strtod(latitude_str, NULL);
            double longitude_double = strtod(longitude_str, NULL);
            
            cJSON *city_object = cJSON_CreateObject();
            cJSON_AddStringToObject(city_object, "display_name", display_name_str);
            cJSON_AddStringToObject(city_object, "latitude_string", latitude_str);
            cJSON_AddNumberToObject(city_object, "latitude", latitude_double, false);
            cJSON_AddStringToObject(city_object, "longitude_string", longitude_str);
            cJSON_AddNumberToObject(city_object, "longitude", longitude_double, false);
            cJSON_AddItemToArray(return_object, city_object);
        }
    }
        
    char* result_string = cJSON_Print(return_object);
    if (parsed_json != NULL)
        cJSON_Delete(parsed_json);
        
    if (return_object != NULL)
        cJSON_Delete(return_object);
    
    return result_string;
}