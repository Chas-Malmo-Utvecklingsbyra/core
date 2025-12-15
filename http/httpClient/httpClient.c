#include "httpClient.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../utils/min.h"
#include "../../http/parser.h"

/* ======================== CALLBACKS ======================== */
void HTTP_TCP_Client_Callback_On_Connect(TCP_Client *client);
void HTTP_TCP_Client_Callback_On_Disconnect (TCP_Client *client);
void HTTP_TCP_Client_Callback_On_Received_Bytes_From_Server (TCP_Client *client, const uint8_t *buffer, const uint32_t buffer_size);
void HTTP_TCP_Client_Callback_On_Full_Request (TCP_Client *client, Http_Request *request);
void HTTP_TCP_Client_Callback_On_Error (TCP_Client *client, TCP_Client_Result error);


bool HTTPClient_Work(void* _Context)
{
	HTTPClient* client = (HTTPClient*)_Context;
    (void)client;
    // printf("hello from inside httpclient_work\n");
    TCP_Client_Result result = tcp_client_work(&client->tcp_client);

    if(result != TCP_Client_Result_OK){
        printf("Result from tcp client work not OK: %d\n", result);
        return true;
    }

    switch(client->state){
        case HTTPClient_State_Init:
            
            client->state = HTTPClient_State_Connect;
            break;
        case HTTPClient_State_Connect:
            client->state = HTTPClient_State_Transmit;
            break;
        case HTTPClient_State_Transmit:
            /* client->state = HTTPClient_State_Receive; */
            
            if(client->tcp_client.server.outgoing_buffer_bytes >= strlen((char*)client->buffer)){
                client->state = HTTPClient_State_Receive;
            }
            break;
        case HTTPClient_State_Receive:
            // printf("inside receive stage\n");



            /* client->state = HTTPClient_State_Close; */
            break;
        case HTTPClient_State_Close:
            /* printf("Done!\n"); */
            return true;

            break;
    }
    return false;
	
}

int HTTPClient_Initiate(HTTPClient* _Client, HTTPClient_Callback_On_Received_Full_Message on_received_full_message)
{
	memset(_Client, 0, sizeof(HTTPClient));
	
	_Client->buffer = NULL;
    memset(_Client->inbuffer, 0, sizeof(_Client->inbuffer));
    _Client->on_received_full_message = on_received_full_message;
    
    TCP_Client_Result tcp_client_result = tcp_client_init(&_Client->tcp_client, (void*)_Client, HTTP_TCP_Client_Callback_On_Received_Bytes_From_Server, HTTP_TCP_Client_Callback_On_Full_Request, HTTP_TCP_Client_Callback_On_Connect, HTTP_TCP_Client_Callback_On_Disconnect, HTTP_TCP_Client_Callback_On_Error);
	if(tcp_client_result != TCP_Client_Result_OK){
        printf("TCP client init Error: %d\n", tcp_client_result);
        return -1;
    }
    return 0;
}

int HTTPClient_GET(HTTPClient* _Client, const char* _URL, const char *route)
{
	_Client->buffer = malloc(4096);
	if(_Client->buffer == NULL)
		return -1;

	snprintf((char*)_Client->buffer, 4096, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", route, _URL);

	_Client->bufferPtr = _Client->buffer;
    
    const char *IP = NULL;
    
    //temp until we implement a dns-function --- TODO
    IP = _URL;

    // if(domain) lookup IP --- send IP to next row

    if(IP == NULL){
        printf("Failed to get IP\n");
        return -1;
    }
    
    TCP_Client_Result tcp_client_result = tcp_client_connect(&_Client->tcp_client, IP, 10280);

    if (tcp_client_result != TCP_Client_Result_OK){
        printf("TCP Client Connect Error: %d\n", tcp_client_result);
        return -1;
    }

    return 0;
}

void HTTP_TCP_Client_Callback_On_Connect(TCP_Client *client){
    (void)client;
    HTTPClient* http_client = (HTTPClient*)client->context;
    
    printf("[CLIENT] I successfully connected!\n");
    http_client->state = HTTPClient_State_Transmit;
    tcp_client_send(client, &http_client->buffer[0], strlen((char*)http_client->buffer));
}

void HTTP_TCP_Client_Callback_On_Disconnect (TCP_Client *client){
    (void)client;

    printf("[CLIENT] I am not connected anymore!\n");
}

void HTTP_TCP_Client_Callback_On_Received_Bytes_From_Server (TCP_Client *client, const uint8_t *buffer, const uint32_t buffer_size){
    (void)client;

    /* printf("[CLIENT] Received: %.*s\n", (int)buffer_size, buffer); */


    HTTPClient* http_client = (HTTPClient*)client->context;
    
    memcpy(http_client->inbuffer, buffer, min_uint32(sizeof(http_client->inbuffer), buffer_size));

    // is the message complete? if not continue to read. Cause now we break at the first message.
    http_client->on_received_full_message(http_client); // will crash if NULL - FIX later (PL)
    
    http_client->state = HTTPClient_State_Close;
}

void HTTP_TCP_Client_Callback_On_Full_Request (TCP_Client *client, Http_Request *request){
    (void)client;

    printf("Received full request: %s %s\n", Http_Request_Get_Method_String(request), request->start_line.path);    
}

void HTTP_TCP_Client_Callback_On_Error (TCP_Client *client, TCP_Client_Result error){
    (void)client;

    printf("[CLIENT] ERROR!\nI encountered the following error: %d\n", error);
}

void HTTPClient_Reset(HTTPClient *_Client)
{
    if(_Client->buffer != NULL){
        free(_Client->buffer);
        _Client->buffer = NULL;
    }
    
    _Client->bufferPtr = NULL;
    memset(_Client->inbuffer, 0, sizeof(_Client->inbuffer));
    
    _Client->lat = 0.0f;
    _Client->lon = 0.0f;
    _Client->done = false;

    _Client->state = HTTPClient_State_Init;

    tcp_client_disconnect(&_Client->tcp_client);

    _Client->tcp_client.server.incoming_buffer_bytes = 0;
    _Client->tcp_client.server.outgoing_buffer_bytes = 0;
    memset(_Client->tcp_client.server.incoming_buffer, 0, TCP_CLIENT_RECEIVE_BUFFER_SIZE);
    memset(_Client->tcp_client.server.outgoing_buffer, 0, TCP_CLIENT_OUTGOING_BUFFER_SIZE);
    
    _Client->tcp_client.server.close_requested = false;
}

void HTTPClient_Dispose(HTTPClient* _Client)
{
	if(_Client->buffer != NULL){
        free(_Client->buffer);
        _Client->buffer = NULL;
    }

    tcp_client_disconnect(&_Client->tcp_client);

    memset(_Client->inbuffer, 0, sizeof(_Client->inbuffer));
    _Client->bufferPtr = NULL;

    _Client->lat = 0.0f;
    _Client->lon = 0.0f;
    _Client->done = false;

    _Client->state = HTTPClient_State_Init;

    _Client->tcp_client.server.incoming_buffer_bytes = 0;
    _Client->tcp_client.server.outgoing_buffer_bytes = 0;
    
}