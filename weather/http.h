#ifndef HTTP_H
#define HTTP_H

#include <curl/curl.h>

typedef struct {
    CURL* curl;
} Http;

typedef struct 
{
    char* data;
    int size;
} Http_Response;

int Http_Initialize(Http* h);
int Http_Perform(Http* h, const char* data, Http_Response* response);
void Http_Dispose_Response(Http_Response* response);
void Http_Dispose(Http* h);


/* Initialize */
/* Dispose */
/* Perform */

#endif