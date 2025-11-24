#include "json_config.h"
#include <string.h>
#include <stdio.h>
#include "../fileHelper/fileHelper.h"
#include "../../string/strdup.h"

/**
 * @brief Maps a field name string to its corresponding Json_Config_Field_Enum value.
 * 
 * @param field_name The name of the configuration field.
 * @return Corresponding Json_Config_Field_Enum value, or -1 if not found.
 */
Json_Config_Field_Enum get_json_field_name_enum(const char* field_name)
{
    if (strcmp(field_name, "server_host") == 0) return Json_Config_Field_Server_Host;
    else if (strcmp(field_name, "server_port") == 0) return Json_Config_Field_Server_Port;
    else if (strcmp(field_name, "debug") == 0) return Json_Config_Field_Debug;
    else if (strcmp(field_name, "max_connections") == 0) return Json_Config_Field_Max_Connections;
    else if (strcmp(field_name, "postgresql_host") == 0) return Json_Config_Field_Postgresql_Host;
    else if (strcmp(field_name, "postgresql_api_key") == 0) return Json_Config_Field_Postgresql_Api_Key;
    
    return -1;
}

/**
 * @brief Validates a JSON configuration field based on its name and expected type/constraints.
 * 
 * Checks if the JSON item matches the expected type and constraints for the given field name.
 * Validates string lengths, numeric ranges, and boolean types according to the configuration
 * requirements defined in config.h.
 * 
 * @param field_enum The enum value of the configuration field to validate.
 * @param item The cJSON item to validate.
 * @note TODO: LS - change to a switch statement.
 * @return true if the field is valid according to its constraints, false otherwise.
 * 
 * @note Supported fields:
 *       - "server_host": String, max length CONFIG_MAX_LENGTH_SERVER_HOST
 *       - "server_port": Number, range 0-65535, max string length CONFIG_MAX_LENGTH_SERVER_PORT
 *       - "debug": Boolean
 *       - "max_connections": Number, non-negative
 *       - "postgresql_host": String, max length CONFIG_MAX_LENGTH_POSTGRESQL_HOST
 *       - "postgresql_api_key": String, max length CONFIG_MAX_LENGTH_POSTGRESQL_API_KEY
 */
bool validate_json_field(Json_Config_Field_Enum field_enum, const cJSON* item)
{
    if (item == NULL) return false;
    
    switch (field_enum)
    {
        /* JSON Config Fields */
        case Json_Config_Field_Server_Host:
            if (cJSON_IsString(item) && item->valuestring != NULL && (strlen(item->valuestring) <= CONFIG_MAX_LENGTH_SERVER_HOST)) return true;
            break;
        
        case Json_Config_Field_Server_Port:
            if (cJSON_IsNumber(item) && (item->valueint > 0) && (item->valueint <= CONFIG_MAX_VALUE_SERVER_PORT)) return true;
            break;
            
        case Json_Config_Field_Debug:
            if (cJSON_IsBool(item)) return true;
            break;
            
        case Json_Config_Field_Max_Connections:
            if (cJSON_IsNumber(item) && (item->valueint > 0) && (item->valueint <= CONFIG_MAX_CONNECTIONS_COUNT)) return true;
            break;
            
        case Json_Config_Field_Postgresql_Host:
            if (cJSON_IsString(item) && item->valuestring != NULL && (strlen(item->valuestring) <= CONFIG_MAX_LENGTH_POSTGRESQL_HOST)) return true;
            break;
            
        case Json_Config_Field_Postgresql_Api_Key:
            if (cJSON_IsString(item) && item->valuestring != NULL && (strlen(item->valuestring) <= CONFIG_MAX_LENGTH_POSTGRESQL_API_KEY)) return true;
            break;
            
        default:
            break;
    }
    return false;
}

/**
 * @brief Builds a Config_t structure from a JSON configuration file.
 * 
 * Reads the JSON configuration file, validates each field, and populates
 * the provided Config_t structure with the corresponding values.
 * 
 * @param cfg Pointer to the Config_t structure to populate.
 * @param config_file_path Path to the JSON configuration file.
 * @return Config_Result indicating success or the type of error encountered.
 */
Config_Result parse_json_to_config(Config_t* cfg, const char* config_file_path)
{
    if(cfg == NULL) return Config_Result_Error;
    
    cJSON* root = json_read_from_file(config_file_path);
    
    if (root == NULL) return Config_File_Path_Error;
    
    cJSON* current_element = NULL;
    int i = 0;
    char* field_name = NULL;
    
    for(i = 0; i < cJSON_GetArraySize(root); i++)
    {
        current_element = cJSON_GetArrayItem(root, i);
        if (current_element == NULL) break;
        
        field_name = current_element->string;
        
        if(field_name == NULL)
        {
            current_element = NULL;
            cJSON_Delete(root);
            return Config_Result_Validation_Error;
        }
        
        Json_Config_Field_Enum field_enum = get_json_field_name_enum(field_name);
        
        if(validate_json_field(field_enum, current_element))
        {
            switch (field_enum)
            {
                case Json_Config_Field_Server_Host:
                    cfg->config_server_host = strdup(current_element->valuestring);
                    break;
                
                case Json_Config_Field_Server_Port:
                    cfg->config_server_port = (uint16_t)(current_element->valueint);
                    break;
                
                case Json_Config_Field_Debug:
                    cfg->config_debug = cJSON_IsTrue(current_element);
                    break;
                
                case Json_Config_Field_Max_Connections:
                    cfg->config_max_connections = (size_t)(current_element->valueint);
                    break;
                
                case Json_Config_Field_Postgresql_Host:
                    cfg->config_postgresql_host = strdup(current_element->valuestring);
                    break;
                
                case Json_Config_Field_Postgresql_Api_Key:
                    cfg->config_postgresql_api_key = strdup(current_element->valuestring);
                    break;
                
                default:
                    break;
            }
        }
        else
        {
            current_element = NULL;
            cJSON_Delete(root);
            return Config_Result_Validation_Error;
        }
        
        
    }
    return Config_Result_OK;
}