#include "httpClient.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../utils/min.h"

/* ======================== CALLBACKS ======================== */
void HTTP_TCP_Client_Callback_On_Connect(TCP_Client *client);
void HTTP_TCP_Client_Callback_On_Disconnect (TCP_Client *client);
void HTTP_TCP_Client_Callback_On_Received_Bytes_From_Server (TCP_Client *client, const uint8_t *buffer, const uint32_t buffer_size);
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
            printf("Done!\n");
            return true;

            break;
    }
    return false;
	
}

int HTTPClient_Initiate(HTTPClient* _Client)
{
	memset(_Client, 0, sizeof(HTTPClient));
	
	_Client->buffer = NULL;
    memset(_Client->inbuffer, 0, sizeof(_Client->inbuffer));
    
    TCP_Client_Result tcp_client_result = tcp_client_init(&_Client->tcp_client, (void*)_Client, HTTP_TCP_Client_Callback_On_Received_Bytes_From_Server, HTTP_TCP_Client_Callback_On_Connect, HTTP_TCP_Client_Callback_On_Disconnect, HTTP_TCP_Client_Callback_On_Error);
	if(tcp_client_result != TCP_Client_Result_OK){
        printf("TCP client init Error: %d\n", tcp_client_result);
        return -1;
    }
    return 0;
}

int HTTPClient_GET(HTTPClient* _Client, const char* _URL)
{
	_Client->buffer = malloc(4096);
	if(_Client->buffer == NULL)
		return -1;


	snprintf((char*)_Client->buffer, 4096, "GET %s HTTP/1.1\r\nHost: malmo.onvo.se:81\r\nConnection: close\r\n\r\n", _URL);

	_Client->bufferPtr = _Client->buffer;

    TCP_Client_Result tcp_client_result = tcp_client_connect(&_Client->tcp_client, "142.251.9.102", 80);

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

    printf("[CLIENT] Received: %.*s\n", (int)buffer_size, buffer);


    HTTPClient* http_client = (HTTPClient*)client->context;
    
    memcpy(http_client->inbuffer, buffer, min_uint32(sizeof(http_client->inbuffer), buffer_size));
    http_client->state = HTTPClient_State_Close;
}

void HTTP_TCP_Client_Callback_On_Error (TCP_Client *client, TCP_Client_Result error){
    (void)client;

    printf("[CLIENT] ERROR!\nI encountered the following error: %d\n", error);
}






void HTTPClient_Dispose(HTTPClient* _Client)
{

	if(_Client->buffer != NULL)
		free(_Client->buffer); 

}

