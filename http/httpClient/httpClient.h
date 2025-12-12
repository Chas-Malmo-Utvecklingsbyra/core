#ifndef HTTPClient_h
#define HTTPClient_h

#include <stdint.h>
#include <stdlib.h>
#include "../../tcp/tcp_client/tcp_client.h"

typedef struct HTTPClient HTTPClient;
typedef void(*HTTPClient_Callback_On_Received_Full_Message)(HTTPClient *client);


typedef enum
{
	HTTPClient_State_Init,
	HTTPClient_State_Connect,
	HTTPClient_State_Transmit,
	HTTPClient_State_Receive,
	HTTPClient_State_Close

} HTTPClient_State;


struct HTTPClient
{
	bool done;

	uint8_t* buffer;
	uint8_t* bufferPtr;
	
    uint8_t inbuffer[4096];

	float lat;
	float lon;

	char city_name[64];

    TCP_Client tcp_client; 
    HTTPClient_State state;

	HTTPClient_Callback_On_Received_Full_Message on_received_full_message;


};



int HTTPClient_Initiate(HTTPClient* _Client, HTTPClient_Callback_On_Received_Full_Message on_received_full_message);

int HTTPClient_GET(HTTPClient* _Client, const char* _URL, const char *route);

bool HTTPClient_Work(void* _Context);

void HTTPClient_Reset(HTTPClient *_Client);

void HTTPClient_Dispose(HTTPClient* _Client);



#endif //HTTPClient_h