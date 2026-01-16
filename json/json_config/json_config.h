#ifndef JSON_CONFIG_H
#define JSON_CONFIG_H

#include "../../config/config.h"

/**
 * @brief Parses a JSON configuration file and populate provided Config_t structure.
 * 
 * Reads the JSON configuration file, loads, and populates
 * the provided Config_t structure with the corresponding key-values.
 * 
 * @param cfg Pointer to the Config_t structure to populate.
 * @param config_file_path Path to the JSON configuration file.
 * @return Config_Result indicating success or the type of error encountered.
 */
Config_Result Config_Parse_Json(Config_t* cfg, const char* config_file_path);

#endif /* JSON_CONFIG_H */