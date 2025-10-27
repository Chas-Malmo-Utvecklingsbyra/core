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
int http_get(Http* h, const char* data, Http_Response* response);

/* return: -1, -2 for error, 0 is succesful. headers may be set to NULL if not included in post request. */
int http_post(Http* h, const char* url, char* postData, struct curl_slist* headers);
void http_dispose_response(Http_Response* response);
void http_dispose(Http* h);

#endif