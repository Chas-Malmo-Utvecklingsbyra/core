#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "http_parser.h"

/*
EXAMPLE OF HTTP TEXT:

GET /weather?city=Stockholm HTTP/1.1\r\n
Host: 127.0.0.1\r\n
User-Agent: curl/8.0\r\n
Content-Length: 9\r\n\r\n
BODY TEXT\r\n

TO HELP MAKE PARSER LOGIC.
*/

int http_parse(const char* _InBuffer, size_t _InBufferSize, HttpRequestBlob* _HttpBlob)
{
    HttpRequestBlob* httpData = (HttpRequestBlob*)malloc(sizeof(HttpRequestBlob));
    memset(httpData, 0, sizeof(httpData));

    char* dataCopy = strdup(_InBuffer);
    char* ptr;

    char* pch; 

    char* meth = NULL;
    char* path = NULL;
    char* vers = NULL;

    
    pch = strtok(dataCopy, "\r\n");
    while (pch != NULL)
    {
        ptr = pch;
        do{
            if(httpData->method != NULL && httpData->path != NULL && httpData->version != NULL){
                /* Identify headers and parse KEY and VALUE */
                char* keyPart = strchr(pch, ':');
                if(keyPart) {
                    *keyPart = '\0';
                    char* keyName = pch;
                    char* valueName = keyPart +1;
                    
                    while(*valueName == ' ')
                    valueName++;
                    
                    Header* newHeader = (Header*)malloc(sizeof(Header));
                    memset(newHeader, 0, sizeof(Header));

                    newHeader->keyName = strdup(keyName);
                    newHeader->KeyValue = strdup(valueName);
                    newHeader->next = NULL;

                    if(httpData->Headers == NULL)
                    {
                        httpData->Headers = newHeader;
                    }else{
                        Header* lastHdr = httpData->Headers;
                        while(lastHdr->next)
                            lastHdr = lastHdr->next;
                        lastHdr->next = newHeader;
                    }
                    
                }
            }

            /* obtain request data METHOD PATH VERSION */
            if(meth == NULL)
            {
                meth = ptr;
            }
            else if(path == NULL)
            {
                if(*(ptr) == ' ')
                {
                    path = ptr +1;
                    *(ptr) = '\0';
                }
            }
            else if(vers == NULL)
            {
                if(*(ptr) == ' '){
                    vers = ptr +1;
                    *(ptr) = '\0';
                    /* i want to str dup these */
                    httpData->method = strdup(meth);
                    httpData->path = strdup(path);
                    httpData->version = strdup(vers);
                    break;
                }
            }
           
            ptr++;
        }while (*(ptr) != '\0');

        pch = strtok(NULL, "\r\n");
    }

    char* bodyStart = strstr(_InBuffer, "\r\n\r\n");
    if(bodyStart != NULL)
    {
        bodyStart += 4; /* move past delimiter */
        httpData->body = strdup(bodyStart);
    }    



    printf("httpdata method  : %s\n", httpData->method);
    printf("httpdata path    : %s\n", httpData->path);
    printf("httpdata version : %s\n", httpData->version);

    struct Header* hdr;
    for(hdr = httpData->Headers; hdr != NULL; hdr = hdr->next)
    {
        printf("Header Key Name  : %s\n", hdr->keyName);
        printf("header Key Value : %s\n", hdr->KeyValue);
    }

    printf("Http body        : %s", httpData->body);

    free(dataCopy);
    ptr = NULL;
    http_destroyBlob(httpData);

    return 0;
}

int http_parseBody();

int http_parseHeader();


int http_destroyBlob(HttpRequestBlob* _Blob){

    if(_Blob == NULL)
        return 0;

    free(_Blob->method);
    free(_Blob->path);
    free(_Blob->version);
    free(_Blob->body);

    Header* lastHdr = _Blob->Headers;
    while(lastHdr != NULL){

        Header* next = lastHdr->next;
        free(lastHdr->keyName);
        free(lastHdr->KeyValue);
        free(lastHdr);
        lastHdr = next;
    }
    free(_Blob);


    return 0;
}


int main(){

    const char* httpStr = "GET /weather?city=Stockholm HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/8.0\r\nContent-Length: 9\r\n\r\nBODY TEXT\r\n";

    http_parse(httpStr, 0, NULL);

    return 0;
}
