#include "json_config.h"
#include <string.h>
#include <stdio.h>
#include "../fileHelper/fileHelper.h"
#include "../../string/strdup.h"

Config_Result Config_Parse_Json(Config_t* cfg, const char* config_file_path)
{
    if(!cfg || !config_file_path) return Config_Result_Error;
    
    cJSON* root = json_read_from_file(config_file_path);
    
    if (!root) return Config_Result_Reading_Error;
    
    int root_size = cJSON_GetArraySize(root);
    if(root_size <= 0)
    {
        cJSON_Delete(root);
        return Config_Result_Reading_Error;
    }
    
    if (Config_Fields_Init(cfg, root_size) != Config_Result_OK) // initialize config fields
    {
        cJSON_Delete(root);
        return Config_Result_Error;
    }
    
    cJSON *current_element = NULL;
    char *config_key = NULL;
    
    for (int i = 0; i < root_size; i++)
    {
        current_element = cJSON_GetArrayItem(root, i);
        
        if (!current_element) break;
        
        config_key = current_element->string; // get json key
        
        if (!config_key || !current_element)
        {
            current_element = NULL;
            cJSON_Delete(root);
            return Config_Result_Validation_Error;
        }
        
        Config_Result result = Config_Result_OK;
        if (cJSON_IsString(current_element))
        {
            result = Config_Add_Field(cfg, config_key, Config_Field_Type_String, current_element->valuestring);
        }
        else if (cJSON_IsNumber(current_element))
        {
            result = Config_Add_Field(cfg, config_key, Config_Field_Type_Integer, &(current_element->valueint));
        }
        else if (cJSON_IsBool(current_element))
        {
            bool bool_value = cJSON_IsTrue(current_element) ? true : false;
            result = Config_Add_Field(cfg, config_key, Config_Field_Type_Boolean, &bool_value);
        }
        else if(cJSON_IsArray(current_element) || cJSON_IsObject(current_element))
        {
            size_t array_size = cJSON_GetArraySize(current_element);
            if (array_size == 0)
            {
                current_element = NULL;
                printf("Error: Array/Object field '%s' is empty.\n", config_key);
                cJSON_Delete(root);
                return Config_Result_Validation_Error;
            }
            for (size_t j = 0; j < array_size; j++)
            {
                cJSON *array_item = cJSON_GetArrayItem(current_element, j);
                if (!array_item || !cJSON_IsString(array_item)) // For simplicity, we only support arrays of strings currently.
                {
                    current_element = NULL;
                    printf("Error: Array/Object field '%s' contains invalid item at index %zu.\n", config_key, j);
                    cJSON_Delete(root);
                    return Config_Result_Validation_Error;
                }
                result = Config_Add_Array_Field(cfg, config_key, array_item->valuestring);
            }
        }
        else
        {
            cJSON_Delete(root);
            return Config_Result_Reading_Error;
        }
        if (result != Config_Result_OK)
        {
            cJSON_Delete(root);
            return result;
        }
        
        current_element = NULL;
        config_key = NULL;
    }
    cJSON_Delete(root);
    return Config_Result_OK;
}