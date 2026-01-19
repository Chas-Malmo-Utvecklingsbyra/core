#include "http_server.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "http_router/http_router.h"

void send_response_to_client(TCP_Server *server, TCP_Server_Client *client, const char *response_string, const Http_Content_Type type, const HTTP_Status_Code code)
{
    uint8_t buffer[TCP_MAX_CLIENT_BUFFER_SIZE];
    uint32_t size = 0;
    
    assert(response_string != NULL); /* Should never be NULL here */
    
    http_create_response(buffer, sizeof(buffer), response_string, strlen(response_string), code, &size, type);
    
    TCP_Server_Result send_result = tcp_server_send_to_client(server, client, buffer, size);
    if (send_result != TCP_Server_Result_OK)
    {
        printf("Error on client send\n");
    }

}

void on_received_bytes_from_client(void *context, TCP_Server *server, TCP_Server_Client *client, const uint8_t *buffer, const uint32_t buffer_size) 
{
    HTTP_Server* http_server = (HTTP_Server*)context;
    /* TODO: SS - Try to parse the contents of the request buffer as a HTTP-request. */
    printf("Received %u bytes from client:\n", buffer_size);
    
    Http_Request* httpblob =  Http_Parser_Parse((const char*)buffer);
    if(httpblob == NULL)
    {
        send_response_to_client(server, client, "<h1>Invalid HTTP Request</h1>", HTTP_CONTENT_TYPE_HTML, HTTP_STATUS_CODE_BAD_REQUEST);
        return;
    }

    Request_Handler_Response_t request_handler_response = Http_Router_Handle_Request(&http_server->route_registry, httpblob);

    assert(request_handler_response.response_data != NULL);

    send_response_to_client(server, client, request_handler_response.response_data, request_handler_response.content_type, request_handler_response.status_code);

    Http_Router_Dispose_Response(&request_handler_response);
    Http_Parser_Cleanup(&httpblob);
}

bool HTTP_Server_Initialize(HTTP_Server* http_server, size_t max_connections)
{
    /* TODO: HW - Use this */
    (void)max_connections;
    memset(http_server, 0, sizeof(HTTP_Server));

    if (Route_Registry_Create(&http_server->route_registry) == false)
    {
        printf("Failed to create route registry.\n");
        return false;
    }
    
    TCP_Server_Result server_init_result = tcp_server_init(
            &http_server->tcp_server,
            (void*)http_server,
            &on_received_bytes_from_client);

    if(server_init_result != TCP_Server_Result_OK) 
    {
        printf("Failed to initialize TCP server. Result: %i.\n", server_init_result); // TODO: SS - tcp_server_get_result_as_string(server_init_result)
        return false;
    }

    return true;
}

bool HTTP_Server_Register_Route(HTTP_Server *http_server, const char *route, Http_Method method, RouteHandler handler)
{
    if(Route_Registry_Register(&http_server->route_registry, route, method, handler, NULL))
    {
        return true;
    }

    return false;
}


bool HTTP_Server_Start(HTTP_Server* http_server, uint16_t port)
{
    if (http_server->port != 0)
    {
        printf("Server has not yet been initialized\n");
        return false;
    }
    
    http_server->port = port;

    TCP_Server_Result start_server_result = tcp_server_start(&http_server->tcp_server, http_server->port);
    if(start_server_result != TCP_Server_Result_OK) 
    {
        printf("Failed to start TCP server. Result: %i.\n", start_server_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result)
        return false;
    }
    printf("Server running.\n");

    return true;
}

void HTTP_Server_Work(HTTP_Server* http_server)
{
    
    // TEMP: SS - tcp_server_is_running(server)?

    TCP_Server_Result work_result = tcp_server_work(&http_server->tcp_server);

    switch(work_result){
        case TCP_Server_Result_OK:
        {
            break;
        }
        default: 
        {
            printf("Something went wrong\n");
            break;
        }
    }
}

void HTTP_Server_Dispose(HTTP_Server* http_server)
{
    assert(http_server != NULL);
    Route_Registry_Dispose(&http_server->route_registry);
    tcp_server_dispose(&http_server->tcp_server);
    http_server->port = 0;
}