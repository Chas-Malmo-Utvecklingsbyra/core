#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define CONFIG_DEFAULT_FILE_PATH "settings.json" /* Default config file path */

typedef struct Config_Field_t Config_Field_t;
typedef struct Config_t Config_t;
typedef enum Config_Result
{
    Config_Result_Initialization_Error = -5,
    Config_Result_Allocation_Error = -4,
    Config_Result_Validation_Error = -3,
    Config_Result_Reading_Error = -2,
    Config_Result_Error = -1,
    Config_Result_OK = 0
} Config_Result;

typedef enum Config_Field_Type_t
{
    Config_Field_Type_String,
    Config_Field_Type_Integer,
    Config_Field_Type_Boolean
} Config_Field_Type_t;

struct Config_Field_t
{
    char* config_key;
    void* config_value;
    Config_Field_Type_t field_type;
};

struct Config_t
{
    Config_Field_t* config_fields;
    size_t config_field_count;
};

/**
 * @brief Retrieves or initializes the singleton instance of the configuration.
 * 
 * @param config_file_path Path to the configuration file, if NULL the default path is used.
 * 
 * @note config_file_path can be NULL for default path usage.
 * 
 * Singleton implementation
 * @return Pointer to the Config_t instance, or NULL on failure.
 */
Config_t* config_get_instance(const char* config_file_path);

/**
 * @brief Initializes the config_fields array in the Config_t structure.
 * 
 * Allocates memory for the config_fields array based on the specified field_capacity.
 * 
 * @param cfg Pointer to the Config_t structure to initialize.
 * @param field_capacity Number of fields to allocate space for.
 * 
 * @return Config_Result indicating success or type of error encountered.
 */
Config_Result config_fields_init(Config_t *cfg, size_t field_capacity);

/**
 * @brief Adds a new field to the configuration.
 * 
 * Allocates and sets a new configuration field in the Config_t structure.
 * 
 * @param cfg Pointer to the Config_t structure.
 * @param config_key Key/name of the configuration field.
 * @param field_type Type of the configuration field (String, Integer, Boolean).
 * @param config_value Pointer to the value of the configuration field.
 * 
 * @return Config_Result indicating success or type of error encountered.
 */
Config_Result config_add_field(Config_t *cfg, const char *config_key, Config_Field_Type_t field_type, void *config_value);

/**
 * @brief Destroys the singleton configuration instance and frees associated memory.
 * 
 * After calling this function, the next call to config_get_instance() will
 * attempt to recreate the singleton.
 */
void config_instance_dispose(void);

/**
 * @brief Gets the last error that occurred during singleton initialization.
 * @return Config_Result indicating the type of error, or Config_Result_OK if no error.
 */
Config_Result config_instance_get_last_error(void);

#endif /* CONFIG_H */