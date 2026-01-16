#include "json_config.h"
#include <string.h>
#include <stdio.h>
#include "../fileHelper/fileHelper.h"
#include "../../string/strdup.h"

Config_Result parse_json_to_config(Config_t* cfg, const char* config_file_path)
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
    
    if (config_fields_init(cfg, root_size) != Config_Result_OK) // initialize config fields
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
            result = config_add_field(cfg, config_key, Config_Field_Type_String, current_element->valuestring);
        }
        else if (cJSON_IsNumber(current_element))
        {
            result = config_add_field(cfg, config_key, Config_Field_Type_Integer, &(current_element->valueint));
        }
        else if (cJSON_IsBool(current_element))
        {
            bool bool_value = cJSON_IsTrue(current_element) ? true : false;
            result = config_add_field(cfg, config_key, Config_Field_Type_Boolean, &bool_value);
        }
        else if(cJSON_IsArray(current_element) || cJSON_IsObject(current_element))
        {
            // Nested objects/arrays are not supported
            perror("Nested JSON objects/arrays are not supported in configuration files.\n");
            continue;
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