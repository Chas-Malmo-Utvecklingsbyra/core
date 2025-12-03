#ifndef HTTPClient_h
#define HTTPClient_h

#include <stdint.h>
#include <stdlib.h>
#include "../../tcp/tcp_client/tcp_client.h"






typedef enum
{
	HTTPClient_State_Init,
	HTTPClient_State_Connect,
	HTTPClient_State_Transmit,
	HTTPClient_State_Receive,
	HTTPClient_State_Close

} HTTPClient_State;


typedef struct
{
	uint8_t* buffer;
	uint8_t* bufferPtr;

    uint8_t inbuffer[4096];


    TCP_Client tcp_client; 
    HTTPClient_State state;

} HTTPClient;



int HTTPClient_Initiate(HTTPClient* _Client);

int HTTPClient_GET(HTTPClient* _Client, const char* _URL);

bool HTTPClient_Work(void* _Context);

void HTTPClient_Dispose(HTTPClient* _Client);



#endif //HTTPClient_h