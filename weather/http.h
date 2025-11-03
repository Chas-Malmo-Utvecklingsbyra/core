#ifndef HTTP_H
#define HTTP_H

#include <curl/curl.h>

typedef enum
{
    HTTP_SUCCESSFUL,
    HTTP_ERROR_FAILED_TO_INITIALIZE,
    HTTP_ERROR_FAILED_TO_PERFORM,
} Http_Error;

typedef struct 
{
    CURL* curl;
} Http;

typedef struct 
{
    char* data;
    int size;
} Http_Response;

/* returns: Http_Error enum*/
Http_Error http_initialize(Http* h);

/* returns: Http_Error enum. Outputs a response. Headers may be set to NULL if not included in get request.*/
Http_Error http_get(Http* h, const char* url, Http_Response* response, struct curl_slist* headers);

/* returns: Http_Error enum. Headers may be set to NULL if not included in post request. */
Http_Error http_post(Http* h, const char* url, char* postData, struct curl_slist* headers);

/* Disposes the response heap memory */
void http_dispose_response(Http_Response* response);

/* Disposes the response heap memory */
void http_dispose(Http* h);

#endif