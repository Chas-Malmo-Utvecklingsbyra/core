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

int http_initialize(Http* h);
int http_perform(Http* h, const char* data, Http_Response* response);
void http_dispose_response(Http_Response* response);
void http_dispose(Http* h);

#endif