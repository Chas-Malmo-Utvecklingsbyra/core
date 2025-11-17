#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#ifndef Config_H
#define Config_H

#define CONFIG_FILE_PATH "settings.json"

/* Defines for validating json config */
#define CONFIG_MAX_LENGTH_SERVER_HOST 16       /* max length for IPv4 address in string form xxx.xxx.xxx.xxx */
#define CONFIG_MAX_LENGTH_SERVER_PORT 5        /* max length for port number in string form 65535 */
#define CONFIG_MAX_LENGTH_POSTGRESQL_HOST 255 
#define CONFIG_MAX_LENGTH_POSTGRESQL_API_KEY 512
#define CONFIG_MAX_CONNECTIONS_COUNT 10

typedef struct Config_t Config_t;

typedef enum 
{
    Config_Result_Validation_Error = -3,
    Config_Result_Reading_Error = -2,
    Config_Result_Error = -1,
    Config_Result_OK = 0
} Config_Result;

/* 
* Config json example 
* settings.json:
* {
*   "server_host" : "127.0.0.1",
*   "server_port" : 80,
*   "debug" : true,
*   "max_connections" : 10,
*   "postgresql_host" : "localhost",
*   "postgresql_api_key" : "asdasdd"
* }
*/

struct Config_t
{
    char *config_server_host;
    uint16_t config_server_port;
    bool config_debug;
    size_t config_max_connections;
    char *config_postgresql_host;
    char *config_postgresql_api_key;
};

Config_Result config_init(Config_t* cfg);

Config_Result config_load(Config_t *cfg);

void config_dispose(Config_t* cfg);

#endif /* Config_H */