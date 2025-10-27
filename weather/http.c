#include "http.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <stdio.h>

static size_t http_response_write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    Http_Response* response = (Http_Response*)userp;
    
    size_t realsize = size * nmemb;

    char* ptr = NULL;
    int totalSize = response->size + realsize + 1;
    if(response->data == NULL) {
        ptr = (char*)malloc(totalSize);
    }
    else {
        ptr = realloc(response->data, totalSize);
    }
    
    if(ptr == NULL) {
        /* Out of memory */
        return 0;
    }

    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = 0; /* null-terminate */

    return realsize;
}

Http_Error http_initialize(Http* h)
{
    h->curl = curl_easy_init();

    if (h->curl == NULL)
    {
        printf("Error initializing curl\n");
        return HTTP_ERROR_FAILED_TO_INITIALIZE;
    }

    return 0;
}

Http_Error http_get(Http* h, const char* url, Http_Response* response)
{
    if (h->curl == NULL)
    {
        printf("Curl failed to Initialize");
        return HTTP_ERROR_FAILED_TO_INITIALIZE;
    }

    curl_easy_setopt(h->curl, CURLOPT_URL, url);
    curl_easy_setopt(h->curl, CURLOPT_WRITEFUNCTION, http_response_write_callback);
    curl_easy_setopt(h->curl, CURLOPT_WRITEDATA, (void*)response);

    CURLcode code = curl_easy_perform(h->curl);

    if (code != CURLE_OK)
    {
        printf("Curl failed to perform\n");
        return HTTP_ERROR_FAILED_TO_PERFORM;
    }

    return HTTP_SUCCESSFUL;
}


Http_Error http_post(Http* h, const char* url, char* postData, struct curl_slist* headers)
{
    if (h->curl == NULL)
    {
        printf("CURL has not been initialized!\n");
        return HTTP_ERROR_FAILED_TO_INITIALIZE;
    }

    curl_easy_setopt(h->curl, CURLOPT_URL, url);

    if (headers)
    {
        curl_easy_setopt(h->curl, CURLOPT_HTTPHEADER, headers);
    }

    curl_easy_setopt(h->curl, CURLOPT_POSTFIELDS, postData);

    CURLcode code = curl_easy_perform(h->curl);
    if (code != CURLE_OK)
    {
        printf("Curl failed to post\n");
        return HTTP_ERROR_FAILED_TO_PERFORM;
    }

    if (headers)
    {
        curl_slist_free_all(headers);
    }
    return HTTP_SUCCESSFUL;
}

void http_dispose_response(Http_Response* response) 
{
    if (response == NULL)
    {
        printf("Response is NULL in Http_Dispose_Response\n");
        return;
    }

    free(response->data);
    response->data = NULL;
}

void http_dispose(Http* h)
{
    curl_easy_cleanup(h->curl);
}