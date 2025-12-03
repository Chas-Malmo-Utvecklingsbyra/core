#include "http.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <stdio.h>
#include "../string/strdup.h"
#include "../config/config.h"

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

/**
 * @brief Populates a curl_slist with provided headers.
 * @param slist_headers Pointer to the curl_slist to populate.
 * @param headers Array of header strings to add to the slist.
 * @return Http_Error indicating success or failure.
 * @note TODO: LS - NOT TESTED YET
 */
Http_Error http_populate_headers(struct curl_slist* slist_headers, const char* headers[])
{
    size_t i = 0;
    while (headers[i] != NULL)
    {
        slist_headers = curl_slist_append(slist_headers, headers[i]);
        i++;
    }
    return HTTP_SUCCESSFUL;
}

Http_Error http_initialize(Http* h)
{
    h->curl = curl_easy_init();

    if (h->curl == NULL)
    {
        printf("Error initializing curl\n");
        return HTTP_ERROR_FAILED_TO_INITIALIZE;
    }

    return HTTP_SUCCESSFUL;
}

/**
 * @brief Performs an HTTP GET request to the specified URL.
 * @param url The URL to send the GET request to.
 * @param response Pointer to a char pointer where the response data will be stored, caller responsible for freeing.
 * @param headers Array of header strings to include in the request, or NULL for no headers
 */
Http_Error http_get(const char *url, char **response, const char *headers[])
{
    bool cfg_debug = config_get_instance(NULL)->config_debug;

    Http h;
    struct curl_slist *slist_headers = NULL;
    
    if (headers != NULL)
    {
        http_populate_headers(slist_headers, headers);
    }

    Http_Response http_response;
    http_response.data = NULL;
    http_response.size = 0;
    
    Http_Error result = http_initialize(&h);
    
    if (result == HTTP_ERROR_FAILED_TO_INITIALIZE)
    {
        if (cfg_debug) 
            printf("Http_Initialize failed in http_get\n");
            
        return HTTP_ERROR_FAILED_TO_INITIALIZE;
    }

    if (h.curl == NULL)
    {
        if (cfg_debug) 
            printf("Curl failed to Initialize");
            
        return HTTP_ERROR_FAILED_TO_INITIALIZE;
    }

    curl_easy_setopt(h.curl, CURLOPT_URL, url);
    curl_easy_setopt(h.curl, CURLOPT_WRITEFUNCTION, http_response_write_callback);
    curl_easy_setopt(h.curl, CURLOPT_WRITEDATA, (void *)&http_response);

    if (headers)
    {
        http_populate_headers(slist_headers, headers);
        curl_easy_setopt(h.curl, CURLOPT_HTTPHEADER, slist_headers);
    }

    CURLcode code = curl_easy_perform(h.curl);
    
    if (cfg_debug) 
        printf("CURL perform returned code: %d\n", code);

    if (code != CURLE_OK)
    {
        if (cfg_debug) 
            printf("Curl failed to perform\n");
        return HTTP_ERROR_FAILED_TO_PERFORM;
    }

    if (http_response.data != NULL)
    {
        if (cfg_debug)
            printf("HTTP response data received (%d bytes):\n%s\n", http_response.size, http_response.data);
            
        (*response) = strdup(http_response.data);
    }

    http_dispose_response(&http_response);
    http_dispose(&h, slist_headers);
    
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

void http_dispose(Http* h, struct curl_slist* headers)
{
    if(headers != NULL) curl_slist_free_all(headers);
    
    if(h->curl) curl_easy_cleanup(h->curl);
}
