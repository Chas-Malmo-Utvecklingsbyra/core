#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdint.h>
#include <stdbool.h>

#include "tcp/server/tcp_server.h"    // TODO Fix this, shouldn't have to be this long
#include "http_router/route_registry/route_registry.h"
#include "http/http.h"

typedef struct
{
    TCP_Server tcp_server;
    uint16_t port;
    RouteRegistry route_registry;    
} HTTP_Server;

// void-pointers?
bool HTTP_Server_Initialize(HTTP_Server* http_server, size_t max_connections, void *context);
bool HTTP_Server_Register_Route(HTTP_Server *http_server, const char *route, Http_Method method, RouteHandler handler);
bool HTTP_Server_Start(HTTP_Server* http_server, uint16_t port);
void HTTP_Server_Work(HTTP_Server* http_server);
void HTTP_Server_Dispose(HTTP_Server* http_server);

#endif