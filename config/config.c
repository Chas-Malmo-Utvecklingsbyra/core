#include "config.h"
#include "../json/fileHelper/fileHelper.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cjson/cJSON.h>

/* ---- Local helper functions begin ---- */

/**
 * @brief Duplicates a string by allocating new memory and copying the content.
 * 
 * This is a local implementation of strdup for portability.
 * Allocates memory for a new string and copies the content from the source.
 * 
 * @param str The source string to duplicate. Can be NULL.
 * @return Pointer to the newly allocated string, or NULL if allocation fails or str is NULL.
 * @note The caller is responsible for freeing the returned string.
 *       Should be replaced with standard strdup if available or moved to a utility file.
 */
static char* config_strdup(const char* str)
{
    if(!str) return NULL;
	char* copy = (char*)malloc(strlen(str) + 1);
	if(copy == NULL) return NULL;
	strcpy(copy, str);
	return copy;
}

/**
 * @brief Validates a JSON configuration field based on its name and expected type/constraints.
 * 
 * Checks if the JSON item matches the expected type and constraints for the given field name.
 * Validates string lengths, numeric ranges, and boolean types according to the configuration
 * requirements defined in config.h.
 * 
 * @param field_name The name of the configuration field to validate.
 * @param item The cJSON item to validate.
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
bool validate_json_field(const char* field_name, cJSON* item)
{
    if (!field_name || !item) return false;
    
    if (strcmp(field_name, "server_host") == 0)
    {
        if (cJSON_IsString(item) && (strlen(item->valuestring) <= CONFIG_MAX_LENGTH_SERVER_HOST))
        {
            return true;
        }
    }
    else if (strcmp(field_name, "server_port") == 0)
    {
        if (cJSON_IsNumber(item) && (item->valueint > 0) && (item->valueint <= 65535))
        {
            char port_str[CONFIG_MAX_LENGTH_SERVER_PORT + 1];
            snprintf(port_str, sizeof(port_str), "%d", item->valueint);
            
            if (strlen(port_str) <= CONFIG_MAX_LENGTH_SERVER_PORT)
            {
                return true;
            }
        }
    }
    else if (strcmp(field_name, "debug") == 0)
    {
        if (cJSON_IsBool(item))
        {
            return true;
        }
    }
    else if (strcmp(field_name, "max_connections") == 0)
    {
        if (cJSON_IsNumber(item) && (item->valueint > 0) && (item->valueint <= CONFIG_MAX_CONNECTIONS_COUNT))
        {
            return true;
        }
    }
    else if (strcmp(field_name, "postgresql_host") == 0)
    {
        if (cJSON_IsString(item) && (strlen(item->valuestring) <= CONFIG_MAX_LENGTH_POSTGRESQL_HOST))
        {
            return true;
        }
    }
    else if (strcmp(field_name, "postgresql_api_key") == 0)
    {
        if (cJSON_IsString(item) && (strlen(item->valuestring) <= CONFIG_MAX_LENGTH_POSTGRESQL_API_KEY))
        {
            return true;
        }
    }
    return false;
}

/* ---- Local helper functions end ---- */

/**
 * @brief Initializes a Config_t structure to default values.
 * 
 * Sets all string pointers to NULL, numeric values to 0, and boolean values to false.
 * This function should be called before loading configuration to ensure a clean state.
 * 
 * @param cfg Pointer to the Config_t structure to initialize.
 * @return Config_Result_OK on success, Config_Result_Error if cfg is NULL.
 */
Config_Result config_init(Config_t *cfg)
{
    printf("Initializing configuration...\n");
    if (!cfg) return Config_Result_Error;
    cfg->config_server_host = NULL;
    cfg->config_server_port = 0;
    cfg->config_debug = false;
    cfg->config_max_connections = 0;
    cfg->config_postgresql_host = NULL;
    cfg->config_postgresql_api_key = NULL;
        
    return Config_Result_OK;
}

/**
 * @brief Loads configuration from the JSON config file into the provided Config_t structure.
 * 
 * @param cfg Pointer to the Config_t structure to populate.
 * @return Config_Result indicating success or the type of error encountered.
 */
Config_Result config_load(Config_t *cfg)
{
    cJSON* root = json_read_from_file(CONFIG_FILE_PATH);
    
    if (root == NULL) return Config_Result_Reading_Error;
    
    cJSON* item = NULL;
    
    /* server_host */
    item = cJSON_GetObjectItemCaseSensitive(root, "server_host");
    if(!validate_json_field("server_host", item))
    {
        cJSON_Delete(root);
        config_dispose(cfg);
        return Config_Result_Validation_Error;
    }
    char* temp_server_host = config_strdup(item->valuestring);
    if (!temp_server_host)
    {
        cJSON_Delete(root);
        config_dispose(cfg);
        return Config_Result_Error;
    }
    
    /* server_port */
    item = cJSON_GetObjectItemCaseSensitive(root, "server_port");
    if(!validate_json_field("server_port", item))
    {
        cJSON_Delete(root);
        config_dispose(cfg);
        return Config_Result_Validation_Error;
    }
    uint16_t temp_server_port = (uint16_t)(item->valueint);
    
    /* debug */
    item = cJSON_GetObjectItemCaseSensitive(root, "debug");
    if(!validate_json_field("debug", item))
    {
        cJSON_Delete(root);
        config_dispose(cfg);
        return Config_Result_Validation_Error;
    }
    bool temp_debug = cJSON_IsTrue(item);
    
    /* max_connections */
    item = cJSON_GetObjectItemCaseSensitive(root, "max_connections");
    if(!validate_json_field("max_connections", item))
    {
        cJSON_Delete(root);
        config_dispose(cfg);
        return Config_Result_Validation_Error;
    }
    size_t temp_max_connections = (size_t)(item->valueint);
    
    /* postgresql_host */
    item = cJSON_GetObjectItemCaseSensitive(root, "postgresql_host");
    if(!validate_json_field("postgresql_host", item))
    {
        cJSON_Delete(root);
        config_dispose(cfg);
        return Config_Result_Validation_Error;
    }
    char* temp_postgresql_host = config_strdup(item->valuestring);
    if (!temp_postgresql_host)
    {
        cJSON_Delete(root);
        config_dispose(cfg);
        return Config_Result_Error;
    }
    
    /* postgresql_api_key */
    item = cJSON_GetObjectItemCaseSensitive(root, "postgresql_api_key");
    if(!validate_json_field("postgresql_api_key", item))
    {
        cJSON_Delete(root);
        config_dispose(cfg);
        return Config_Result_Validation_Error;
    }
    char* temp_postgresql_api_key = config_strdup(item->valuestring);
    if (!temp_postgresql_api_key)
    {
        cJSON_Delete(root);
        config_dispose(cfg);
        return Config_Result_Error;
    }
    
    /* assign to config struct */
    cfg->config_server_host = temp_server_host;
    cfg->config_server_port = temp_server_port;
    cfg->config_debug = temp_debug;
    cfg->config_max_connections = temp_max_connections;
    cfg->config_postgresql_host = temp_postgresql_host;
    cfg->config_postgresql_api_key = temp_postgresql_api_key;
    
    /* clean up */
    cJSON_Delete(root);
    temp_server_host = NULL;
    temp_postgresql_host = NULL;
    temp_postgresql_api_key = NULL;
    
    return Config_Result_OK;
}

/**
 * @brief Frees all dynamically allocated memory in a Config_t structure and resets values.
 * 
 * Deallocates all string fields and sets all pointers to NULL. Resets numeric and boolean
 * fields to their default values (0 and false). This function is safe to call multiple times
 * on the same structure.
 * 
 * @param cfg Pointer to the Config_t structure to dispose. Does nothing if cfg is NULL.
 * 
 * @note After calling this function, the Config_t structure can be safely reused by calling
 *       config_init() followed by config_load().
 */
void config_dispose(Config_t *cfg)
{
    if (!cfg) return;
    
    if (cfg->config_server_host)
    {
        free(cfg->config_server_host);
        cfg->config_server_host = NULL;
    }
    if (cfg->config_postgresql_host)
    {
        free(cfg->config_postgresql_host);
        cfg->config_postgresql_host = NULL;
    }
    if (cfg->config_postgresql_api_key)
    {
        free(cfg->config_postgresql_api_key);
        cfg->config_postgresql_api_key = NULL;
    }
    cfg->config_server_port = 0;
    cfg->config_debug = false;
    cfg->config_max_connections = 0;
}
