#ifndef CORE_HTTP_SERVER_H
#define CORE_HTTP_SERVER_H

#include "../http.h"
#include "../../tcp/server/tcp_server.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef MAX_HTTP_SERVER_ROUTES
#define MAX_HTTP_SERVER_ROUTES 32
#endif

#ifndef MAX_HTTP_SERVER_ROUTE_PATH
#define MAX_HTTP_SERVER_ROUTE_PATH 128
#endif

#ifndef HTTP_QUERY_KEY_MAX_LENGTH
#define HTTP_QUERY_KEY_MAX_LENGTH 128
#endif

#ifndef HTTP_QUERY_VALUE_MAX_LENGTH
#define HTTP_QUERY_VALUE_MAX_LENGTH 128
#endif

typedef struct {
    char key[HTTP_QUERY_KEY_MAX_LENGTH];
    char value[HTTP_QUERY_VALUE_MAX_LENGTH];
} HTTP_Query_Map_Entry;

#ifndef HTTP_MAX_QUERY_ENTRIES
#define HTTP_MAX_QUERY_ENTRIES 32
#endif

typedef struct {
    HTTP_Query_Map_Entry entries[HTTP_MAX_QUERY_ENTRIES];
    uint8_t amount_of_entries;
} HTTP_Query_Map;

bool http_query_map_parse(HTTP_Query_Map *query_map, const char *query_input);
bool http_query_map_get(const HTTP_Query_Map *query_map, const char *key, const char **out_string);

typedef bool (*HTTP_Server_Route_Callback)(HTTP_Method method, const HTTP_Query_Map *query_map, char *out_resonse, uint32_t response_max_buffer_size); // TODO: SS - Make 'query' a map.

typedef struct {
    char path[128];
    HTTP_Server_Route_Callback callback;
} HTTP_Server_Route;

typedef struct {
    TCP_Server tcp_server;
    uint16_t port;

    HTTP_Server_Route routes[MAX_HTTP_SERVER_ROUTES];
    uint32_t amount_of_routes;
} HTTP_Server;

typedef enum {
    HTTP_Server_Result_OK,
    HTTP_Server_Result_Error,
} HTTP_Server_Result;

HTTP_Server_Result http_server_init(HTTP_Server *http_server, uint16_t port);
HTTP_Server_Result http_server_dispose(HTTP_Server *server);

HTTP_Server_Result http_server_work(HTTP_Server *server);

bool http_server_add_route(HTTP_Server *server, const char *path, HTTP_Server_Route_Callback callback);



#endif