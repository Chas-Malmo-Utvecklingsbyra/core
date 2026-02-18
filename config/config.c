#include "config.h"

#include <string.h>
#include <stdio.h>
#include "../json/json_config/json_config.h"
#include "../string/strdup.h"

/* Singleton implementation */
static Config_t* global_config_singleton = NULL;
static bool global_is_config_singleton_initialized = false;
static Config_Result global_config_error = Config_Result_OK;

/* ==== Local functions start ==== */

/**
 * @brief Initializes the Config_t structure with default values.
 * 
 * Sets the config_fields pointer to NULL and config_field_count to 0.
 * 
 * @param cfg Pointer to the Config_t structure to initialize.
 * @return Config_Result indicating success or type of error encountered.
 */
Config_Result Config_Init(Config_t *cfg)
{
    if (!cfg) return Config_Result_Error;
    
    cfg->config_fields = NULL;
    cfg->config_field_count = 0;
    
    return Config_Result_OK;
}

/**
 * @brief Initializes a Config_Field_t structure with default values.
 * 
 * Allocates memory for a Config_Field_t structure and sets its fields to default values.
 * The config_key and config_value pointers are set to NULL, and field_type is set to
 * Config_Field_Type_String by default.
 * 
 * @return Pointer to the initialized Config_Field_t structure, or NULL on memory allocation failure.
 */
Config_Field_t* Config_Field_Init()
{
    Config_Field_t* field = malloc(sizeof(Config_Field_t));
    if (!field) return NULL; // allocation failed
    
    field->config_key = NULL;
    field->config_value = NULL;
    field->field_type = Config_Field_Type_String; // default type
    
    return field;
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
 *       Config_Init() followed by Config_Load().
 */
void Config_Dispose(Config_t *cfg)
{
    if (cfg == NULL) return;
    
    if (cfg->config_fields)
    {
        for (size_t i = 0; i < cfg->config_field_count; i++)
        {
            Config_Field_t* field = cfg->config_fields[i];
            if (!field) continue;
            
            if (field->config_key)
            {
                free(field->config_key);
                field->config_key = NULL;
            }
            if (field->config_value)
            {
                free(field->config_value);
                field->config_value = NULL;
            }
            free(field);
            field = NULL;
        }
        free(cfg->config_fields);
        cfg->config_fields = NULL;
    }
    cfg->config_field_count = 0;
}

/**
 * @brief Loads configuration from the JSON config file into the provided Config_t structure.
 * 
 * @param cfg Pointer to the Config_t structure to populate.
 * @return Config_Result indicating success or the type of error encountered.
 */

Config_Result Config_Load(Config_t *cfg, const char* config_file_path)
{
    if (cfg == NULL) return Config_Result_Initialization_Error;

    /* use default config file path if none provided */
    if (!config_file_path) config_file_path = CONFIG_DEFAULT_FILE_PATH;
    
    Config_Result result = Config_Result_OK;
    result = Config_Parse_Json(cfg, config_file_path);
    
    return result;
}

/* ==== Local functions end ==== */

Config_t* Config_Get_Instance(const char* config_file_path)
{
    if (!global_config_singleton && !global_is_config_singleton_initialized) 
    {
        global_config_singleton = malloc(sizeof(Config_t));
        memset(global_config_singleton, 0, sizeof(Config_t));
        
        if (!global_config_singleton) 
        {
            global_config_error = Config_Result_Error;
            global_is_config_singleton_initialized = true; // Prevent retry
            return NULL;
        }
        
        if (Config_Init(global_config_singleton) != Config_Result_OK)
        {
            free(global_config_singleton);
            global_config_singleton = NULL;
            global_config_error = Config_Result_Error;
            global_is_config_singleton_initialized = true; // Prevent retry
            return NULL;
        }
        if (Config_Load(global_config_singleton, config_file_path) != Config_Result_OK)
        {
            Config_Dispose(global_config_singleton);
            free(global_config_singleton);
            global_config_singleton = NULL;
            global_config_error = Config_Result_Error;
            global_is_config_singleton_initialized = true; // Prevent retry
            return NULL;
        }
        
        global_config_error = Config_Result_OK;
        global_is_config_singleton_initialized = true;
    }
    return global_config_singleton;
}

Config_Result Config_Fields_Init(Config_t *cfg, size_t field_capacity)
{
    if (!cfg || field_capacity <= 0)
        return Config_Result_Error;

    cfg->config_fields = malloc(sizeof(Config_Field_t) * field_capacity);

    if (!cfg->config_fields)
        return Config_Result_Allocation_Error;

    return Config_Result_OK;
}

Config_Result Config_Add_Field(Config_t *cfg, const char *config_key, Config_Field_Type_t field_type, void *config_value)
{
    if (cfg == NULL || config_key == NULL || config_value == NULL) // not initialized
        return Config_Result_Initialization_Error;

    Config_Field_t *new_field = Config_Field_Init();
    if (!new_field)
        return Config_Result_Allocation_Error;

    char *temp_field_name = NULL;
    switch (field_type)
    {
        case Config_Field_Type_String:
            temp_field_name = strdup(config_key);
            if (!temp_field_name) return Config_Result_Allocation_Error;
            
            char *temp_string_value = strdup((char *)config_value);
            if (!temp_string_value)
            {
                free(temp_field_name);
                return Config_Result_Allocation_Error;
            }
        
            new_field->config_key = temp_field_name;
            new_field->config_value = temp_string_value;
            new_field->field_type = Config_Field_Type_String;
            cfg->config_fields[cfg->config_field_count] = new_field;
            cfg->config_field_count++;
            new_field = NULL;  // Ownership transferred
            break;
            
        case Config_Field_Type_Integer:
            temp_field_name = strdup(config_key);
            if (!temp_field_name) return Config_Result_Allocation_Error;
            
            int *temp_int_value = malloc(sizeof(int));
            if (!temp_int_value)
            {
                free(temp_field_name);
                return Config_Result_Allocation_Error;
            }
            
            *temp_int_value = *((int *)config_value);
            new_field->config_key = temp_field_name;
            new_field->config_value = temp_int_value;
            new_field->field_type = Config_Field_Type_Integer;
            cfg->config_fields[cfg->config_field_count] = new_field;
            cfg->config_field_count++;
            new_field = NULL;  // Ownership transferred
            break;
            
        case Config_Field_Type_Boolean:
            temp_field_name = strdup(config_key);
            if (!temp_field_name) return Config_Result_Allocation_Error;
            
            bool *temp_bool_value = malloc(sizeof(bool));
            if (!temp_bool_value)
            {
                free(temp_field_name);
                return Config_Result_Allocation_Error;
            }
            
            *temp_bool_value = *((bool *)config_value);
            new_field->config_key = temp_field_name;
            new_field->config_value = temp_bool_value;
            new_field->field_type = Config_Field_Type_Boolean;
            cfg->config_fields[cfg->config_field_count] = new_field;
            cfg->config_field_count++;
            new_field = NULL;  // Ownership transferred
            break;
            
        default:
            free(new_field);
            return Config_Result_Error;
    }
    return Config_Result_OK;
}

Config_Result Config_Add_Array_Field(Config_t *cfg, const char *config_key, void *array_value)
{
    if (cfg == NULL || config_key == NULL || array_value == NULL) // not initialized
        return Config_Result_Initialization_Error;

    Config_Field_t *new_field = Config_Field_Init();
    if (!new_field)
        return Config_Result_Allocation_Error;

    char *temp_field_name = strdup(config_key);
    if (!temp_field_name)
        return Config_Result_Allocation_Error;

    /* Only supports arrays of strings currently */
    char *temp_string_value = strdup((char *)array_value);
    if (!temp_string_value)
    {
        free(temp_field_name);
        return Config_Result_Allocation_Error;
    }

    new_field->config_key = temp_field_name;
    new_field->config_value = temp_string_value;
    new_field->field_type = Config_Field_Type_String_Array;
    cfg->config_fields[cfg->config_field_count] = new_field;
    cfg->config_field_count++;
    new_field = NULL;

    return Config_Result_OK;
}

char *Config_Get_Field_Value_String(Config_t *cfg, const char *key)
{
    if (!cfg || !key)
        return NULL;
    
    for (size_t i = 0; i < cfg->config_field_count; i++)
    {
        if(strcmp(cfg->config_fields[i]->config_key, key) == 0)
        {
            if (cfg->config_fields[i]->field_type == Config_Field_Type_String)
            {
                return (char *)cfg->config_fields[i]->config_value;
            }
            else
            {
                return NULL; /* Type mismatch */
            }
        }
    }
    return NULL; /* Key not found */
}

char *Config_Get_Field_Value_From_String_Array(Config_t *cfg, const char *key, size_t index)
{
    size_t index_counter = 0;
    if (!cfg || !key)
        return NULL;

    for (size_t i = 0; i < cfg->config_field_count; i++)
    {
        if (strcmp(cfg->config_fields[i]->config_key, key) == 0)
        {
            
            if (cfg->config_fields[i]->field_type == Config_Field_Type_String_Array)
            {
                    if (index_counter == index)
                    {
                        printf("Returning value for key '%s' at index %zu\n", key, index);
                        return (char *)cfg->config_fields[i]->config_value;
                    }
                    else
                    {
                        index_counter++;
                        continue;
                    }
            }
            else
            {
                return NULL; /* Type mismatch */
            }
        }
    }
    return NULL; /* Key not found */
}

int Config_Get_Field_Value_Integer(Config_t *cfg, const char *config_key, bool *out_found)
{
    if (!cfg || !config_key)
    {
        if (out_found) *out_found = false;
        return 0;
    }
    for (size_t i = 0; i < cfg->config_field_count; i++)
    {
        if(strcmp(cfg->config_fields[i]->config_key, config_key) == 0)
        {
            if (cfg->config_fields[i]->field_type == Config_Field_Type_Integer)
            {
                if (out_found) *out_found = true;
                return *((int *)cfg->config_fields[i]->config_value);
            }
            else
            {
                if (out_found) *out_found = false;
                return 0; /* Type mismatch */
            }
        }
    }
    if (out_found) *out_found = false;
    return 0; /* Key not found */
}

bool Config_Get_Field_Value_Boolean(Config_t *cfg, const char *config_key, bool *out_found)
{
    if (!cfg || !config_key)
    {
        if (out_found)
            *out_found = false;
        return 0;
    }
    for (size_t i = 0; i < cfg->config_field_count; i++)
    {
        if (strcmp(cfg->config_fields[i]->config_key, config_key) == 0)
        {
            if (cfg->config_fields[i]->field_type == Config_Field_Type_Boolean)
            {
                if (out_found)
                    *out_found = true;
                return *((bool *)cfg->config_fields[i]->config_value);
            }
            else
            {
                if (out_found)
                    *out_found = false;
                return false; /* Type mismatch */
            }
        }
    }
    if (out_found)
        *out_found = false;
    return 0; /* Key not found */
}

void Config_Instance_Dispose(void)
{
    if (global_config_singleton) 
    {
        Config_Dispose(global_config_singleton);
        free(global_config_singleton);
        global_config_singleton = NULL;
    }
    global_is_config_singleton_initialized = false;
    global_config_error = Config_Result_OK;
}

Config_Result Config_Instance_Get_Last_Error(void)
{
    return global_config_error;
}