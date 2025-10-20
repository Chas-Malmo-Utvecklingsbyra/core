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

int Http_Initialize(Http* h)
{
    h->curl = curl_easy_init();

    if (h->curl == NULL)
    {
        printf("Error initializing curl\n");
        return -1;
    }

    return 0;
}

int Http_Perform(Http* h, const char* data, Http_Response* response)
{
    curl_easy_setopt(h->curl, CURLOPT_URL, data);
    curl_easy_setopt(h->curl, CURLOPT_WRITEFUNCTION, http_response_write_callback);
    curl_easy_setopt(h->curl, CURLOPT_WRITEDATA, (void*)response);

    CURLcode code = curl_easy_perform(h->curl);

    if (code != CURLE_OK)
    {
        printf("Curl failed to perform\n");
        return -1;
    }

    return 0;
}

void Http_Dispose_Response(Http_Response* response) 
{
    if (response == NULL)
    {
        printf("Response is NULL in Http_Dispose_Response\n");
        return;
    }

    free(response->data);
    response->data = NULL;
}

void Http_Dispose(Http* h)
{
    curl_easy_cleanup(h->curl);
}