#include "config.h"
#include "../json/json_config/json_config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Singleton implementation */
static Config_t* global_config_singleton = NULL;
static bool global_is_config_singleton_initialized = false;
static Config_Result global_config_error = Config_Result_OK;

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
    if (cfg == NULL) return;
    
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
    if(cfg->locationiq_access_token)
    {
        free(cfg->locationiq_access_token);
        cfg->locationiq_access_token = NULL;
    }
    cfg->config_server_port = 0;
    cfg->config_debug = false;
    cfg->config_max_connections = 0;
}

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

Config_Result config_load(Config_t *cfg, const char* config_file_path)
{
    if (cfg == NULL) return Config_Result_Error;
    
    /* use default config file path if none provided */
    if (config_file_path == NULL) config_file_path = CONFIG_FILE_PATH;
    
    Config_Result result = Config_Result_OK;
    result = parse_json_to_config(cfg, config_file_path);
    
    return result;
}

/**
 * @brief Retrieves or initializes the singleton instance of the configuration.
 * 
 * @param config_file_path Path to the configuration file, if NULL the default path is used.
 * 
 * @note Send in NULL for config_file_path if it has already been initialized.
 * 
 * Singleton implementation
 * @return Pointer to the Config_t instance, or NULL on failure.
 */
Config_t* config_get_instance(const char* config_file_path)
{
    if (!global_config_singleton && !global_is_config_singleton_initialized) 
    {
        global_config_singleton = malloc(sizeof(Config_t));
        if (!global_config_singleton) 
        {
            global_config_error = Config_Result_Error;
            global_is_config_singleton_initialized = true; // Prevent retry
            return NULL;
        }
        
        Config_Result init_result = config_init(global_config_singleton);
        if(init_result != Config_Result_OK)
        {
            free(global_config_singleton);
            global_config_singleton = NULL;
            global_config_error = init_result;
            global_is_config_singleton_initialized = true; // Prevent retry
            return NULL;
        }
        
        Config_Result load_result = config_load(global_config_singleton, config_file_path);
        if(load_result != Config_Result_OK)
        {
            config_dispose(global_config_singleton);
            free(global_config_singleton);
            global_config_singleton = NULL;
            global_config_error = load_result;
            global_is_config_singleton_initialized = true; // Prevent retry
            return NULL;
        }
        
        global_config_error = Config_Result_OK;
        global_is_config_singleton_initialized = true;
    }
    return global_config_singleton;
}

/**
 * @brief Destroys the singleton configuration instance and frees associated memory.
 * 
 * After calling this function, the next call to config_get_instance() will
 * attempt to recreate the singleton.
 */
void config_instance_dispose(void)
{
    if (global_config_singleton) 
    {
        config_dispose(global_config_singleton);
        free(global_config_singleton);
        global_config_singleton = NULL;
    }
    global_is_config_singleton_initialized = false;
    global_config_error = Config_Result_OK;
}

/**
 * @brief Gets the last error that occurred during singleton initialization.
 * @return Config_Result indicating the type of error, or Config_Result_OK if no error.
 */
Config_Result config_instance_get_last_error(void)
{
    return global_config_error;
}