#ifndef __JSON_CONFIG_H__
#define __JSON_CONFIG_H__

#include "../../config/config.h"

typedef enum Json_Config_Field_Enum
{
    Json_Config_Field_Server_Host,
    Json_Config_Field_Server_Port,
    Json_Config_Field_Debug,
    Json_Config_Field_Max_Connections,
    Json_Config_Field_Postgresql_Host,
    Json_Config_Field_Postgresql_Api_Key,
    Json_Config_Field_Locationiq_Access_Token,
    Json_Config_Field_Allowed_Routes
} Json_Config_Field_Enum;

Config_Result parse_json_to_config(Config_t* cfg, const char* config_file_path);

#endif /* __JSON_CONFIG_H__ */