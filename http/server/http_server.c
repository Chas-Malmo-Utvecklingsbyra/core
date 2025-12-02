#include "http_server.h"
#include "../parser.h"
#include "../http.h"

#define HTTP_RESPONSE_MAX_LENGTH 1024

void on_received_bytes_from_client(void *context, TCP_Server *server, TCP_Server_Client *client, const uint8_t *buffer, const uint32_t buffer_size) {
    HTTP_Server *http_server = (HTTP_Server *)context;
    (void)http_server;

    (void)buffer_size;
    // printf("Received %u bytes from client.\n", buffer_size);

    char response_string[HTTP_RESPONSE_MAX_LENGTH];
    memset(&response_string[0], 0, sizeof(response_string));
    
    Http_Request* httpblob =  Http_Parser_Parse((const char*)buffer); // TODO: SS - Use 'buffer_size' here?
    if(httpblob == NULL){
        snprintf(&response_string[0], HTTP_RESPONSE_MAX_LENGTH, "%s", "<h1>Invalid HTTP Request</h1>");

        uint8_t outgoing_buffer[HTTP_RESPONSE_MAX_LENGTH];
        uint32_t outgoing_size = 0;
        http_create_response(outgoing_buffer, sizeof(outgoing_buffer), response_string, strlen(response_string), &outgoing_size);

        TCP_Server_Result send_result = tcp_server_send_to_client(server, client, outgoing_buffer, outgoing_size);

        if(send_result != TCP_Server_Result_OK)
        {
            printf("Error on client send\n");
        }
        return;
    }
    
    HTTP_Server_Route *found_route = NULL;
    uint16_t i;
    for(i = 0; i < http_server->amount_of_routes; i++) {
        HTTP_Server_Route *route = &http_server->routes[i];
        if(strcmp(httpblob->start_line.path, route->path) == 0) {
            found_route = route;
            break;
        }
    }
    
    // Build the query-map.
    HTTP_Query_Map query_map;
    http_query_map_parse(&query_map, httpblob->start_line.query);

    printf("'%s' '%s', Query (amount: %i).\n", Http_Request_Get_Method_String(httpblob), httpblob->start_line.path, query_map.amount_of_entries);

    HTTP_Method method = HTTP_Method_GET; // TEMP: SS - Hardcoded.
    if(found_route != NULL) {
        if(found_route->callback != NULL) {
            char route_response[HTTP_RESPONSE_MAX_LENGTH];
            memset(&route_response[0], 0, sizeof(route_response));

            bool ok = found_route->callback(method, &query_map, &route_response[0], HTTP_RESPONSE_MAX_LENGTH); // TODO: SS - Return something other than a bool.
            if(!ok) {
                snprintf(&response_string[0], HTTP_RESPONSE_MAX_LENGTH, "%s", "<h1>Route failed.</h1>");
            }
            else {
                snprintf(&response_string[0], HTTP_RESPONSE_MAX_LENGTH, "%s", route_response);
            }
        }
        else {
            snprintf(&response_string[0], HTTP_RESPONSE_MAX_LENGTH, "%s", "<h1>503</h1>");
        }
    }
    else {
        // Send a 404.
        snprintf(&response_string[0], HTTP_RESPONSE_MAX_LENGTH, "%s", "<h1>404</h1>");
    }
    
    uint8_t outgoing_buffer[HTTP_RESPONSE_MAX_LENGTH];
    uint32_t outgoing_size = 0;
    http_create_response(outgoing_buffer, sizeof(outgoing_buffer), response_string, strlen(response_string), &outgoing_size);
    
    TCP_Server_Result send_result = tcp_server_send_to_client(server, client, outgoing_buffer, outgoing_size);

    if (send_result != TCP_Server_Result_OK) {
        printf("Failed to send bytes to client. Send_Result: %i.\n", send_result);
    }

    Http_Parser_Cleanup(httpblob);
}

HTTP_Server_Result http_server_init(HTTP_Server *http_server, uint16_t port) {
    TCP_Server *tcp_server = &http_server->tcp_server;
    memset(tcp_server, 0, sizeof(TCP_Server));

    http_server->port = port;

    memset(&http_server->routes[0], 0, sizeof(HTTP_Server_Route) * MAX_HTTP_SERVER_ROUTES);
    http_server->amount_of_routes = 0;

    TCP_Server_Result server_init_result = tcp_server_init(
        tcp_server,
        http_server->port,
        (void *)http_server,
        &on_received_bytes_from_client
    );
    if(server_init_result != TCP_Server_Result_OK) 
    {
        printf("Failed to initialize TCP server. Result: %i.\n", server_init_result); // TODO: SS - tcp_server_get_result_as_string(server_init_result)
        return HTTP_Server_Result_Error;
    }

    TCP_Server_Result start_server_result = tcp_server_start(tcp_server);
    if(start_server_result != TCP_Server_Result_OK) 
    {
        printf("Failed to start TCP server. Result: %i.\n", start_server_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result)
        return HTTP_Server_Result_Error;
    }
    
    return HTTP_Server_Result_OK;
}

bool http_query_map_parse(HTTP_Query_Map *query_map, const char *query_input) {
    if(query_map == NULL) {
        return false;
    }
    if(query_input == NULL) {
        return false;
    }

    memset(&query_map->entries[0], 0, sizeof(HTTP_Query_Map_Entry) * HTTP_MAX_QUERY_ENTRIES);
    query_map->amount_of_entries = 0;

    char buffer[HTTP_QUERY_KEY_MAX_LENGTH + HTTP_QUERY_VALUE_MAX_LENGTH + 10];
    strncpy(buffer, query_input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *token = strtok(buffer, "&");
    while (token != NULL && query_map->amount_of_entries < HTTP_MAX_QUERY_ENTRIES) {
        HTTP_Query_Map_Entry *entry = &query_map->entries[query_map->amount_of_entries];

        char *equals = strchr(token, '=');
        if (equals) {
            *equals = '\0';
            const char *key = token;
            const char *value = equals + 1;

            strncpy(entry->key, key, HTTP_QUERY_KEY_MAX_LENGTH - 1);
            entry->key[HTTP_QUERY_KEY_MAX_LENGTH - 1] = '\0';

            strncpy(entry->value, value, HTTP_QUERY_VALUE_MAX_LENGTH - 1);
            entry->value[HTTP_QUERY_VALUE_MAX_LENGTH - 1] = '\0';
        } else {
            strncpy(entry->key, token, HTTP_QUERY_KEY_MAX_LENGTH - 1);
            entry->key[HTTP_QUERY_KEY_MAX_LENGTH - 1] = '\0';
            entry->value[0] = '\0';
        }

        query_map->amount_of_entries++;

        token = strtok(NULL, "&");
    }

    return true;
}

bool http_query_map_get(const HTTP_Query_Map *query_map, const char *key, const char **out_string) {
    (void)query_map;
    (void)key;
    *out_string = NULL;

    uint32_t i;
    for(i = 0; i < query_map->amount_of_entries; i++) {
        const HTTP_Query_Map_Entry *entry = &query_map->entries[i];
        if(strcmp(entry->key, key) == 0) {
            *out_string = &entry->value[0];
            return true;
        }
    }
    
    return false;
}

bool http_server_add_route(HTTP_Server *server, const char *path, HTTP_Server_Route_Callback callback) {
    if(server->amount_of_routes >= MAX_HTTP_SERVER_ROUTES) {
        return false;
    }
    if(path == NULL) {
        return false;
    }
    if(callback == NULL) {
        return false;
    }

    HTTP_Server_Route *route = &server->routes[server->amount_of_routes];
    snprintf(&route->path[0], MAX_HTTP_SERVER_ROUTE_PATH, "%s", path);
    route->callback = callback;
    
    server->amount_of_routes += 1;

    return true;
}

HTTP_Server_Result http_server_dispose(HTTP_Server *http_server) {
    tcp_server_dispose(&http_server->tcp_server);
    return HTTP_Server_Result_OK;
}

HTTP_Server_Result http_server_work(HTTP_Server *http_server) {
    TCP_Server_Result work_result = tcp_server_work(&http_server->tcp_server);
    switch(work_result){
        case TCP_Server_Result_OK: {
            return HTTP_Server_Result_OK;
        }
        default: {
            printf("Failed to work the HTTP-server's internal TCP-server. Got result: %i.\n", work_result);
            return HTTP_Server_Result_Error;
        }
    }
    
    return HTTP_Server_Result_Error;
}