#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

/* (PR) The stuff we created last wednesday(?), start of basic http parser */

/* save parsed data into HTTPREQUESTBLOB and HEADER */
typedef struct Header {
    char* keyName; 
    char* KeyValue;
    struct Header *next;
} Header;

typedef struct HttpRequestBlob {
    char* method;
    char* path;
    char* version;
    Header *Headers;
    char* body;
} HttpRequestBlob;


int http_parse(const char* _InBuffer, size_t _InBufferSize, char* _OutBuffer)
{
    HttpRequestBlob* httpData = (HttpRequestBlob*)malloc(sizeof(HttpRequestBlob));
    memset(httpData, 0, sizeof(httpData));

    char* dataCopy = strdup(_InBuffer);
    char* ptr;

    char* pch; 

    /*
    EXAMPLE OF HTTP TEXT:

    GET /weather?city=Stockholm HTTP/1.1\r\n
    Host: 127.0.0.1\r\n
    User-Agent: curl/8.0\r\n
    \r\n

    TO HELP MAKE PARSER LOGIC.
    */

    pch = strtok(dataCopy, "\r\n");

    while (pch != NULL)
    {
        /*printf("%s\n", pch);*/
        ptr = pch;
        do{
            if(httpData->method != NULL && httpData->path != NULL && httpData->version != NULL){
                char* keyPart = strchr(pch, ':');
                if(keyPart) {
                    *keyPart = '\0';
                    char* keyName = pch;
                    char* valueName = keyPart +1;
                    
                    while(*valueName == ' ') /* remove prefix white space */
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

            if(httpData->method == NULL)
            {
                httpData->method = ptr;
            }
            else if(httpData->path == NULL)
            {
                if(*(ptr) == ' ')
                {
                    httpData->path = ptr +1;
                    *(ptr) = '\0';
                }
            }
            else if(httpData->version == NULL)
            {
                if(*(ptr) == ' '){
                    httpData->version = ptr +1;
                    *(ptr) = '\0';
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


    ptr = dataCopy;

    printf("httpdata method  : %s\n", httpData->method);
    printf("httpdata path    : %s\n", httpData->path);
    printf("httpdata version : %s\n", httpData->version);

/* 	struct addrinfo *rp;
	for(rp = res; rp; rp = rp->ai_next) */

    struct Header* hdr;
    for(hdr = httpData->Headers; hdr != NULL; hdr = hdr->next)
    {
        printf("Header Key Name  : %s\n", hdr->keyName);
        printf("header Key Value : %s\n", hdr->KeyValue);
    }

    printf("Http body        : %s", httpData->body);

    return 0;
}

int http_parseBody();

int http_parseHeader();


int main(){

    char* httpStr = "GET /weather?city=Stockholm HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/8.0\r\nContent-Length: 9\r\n\r\nBODY TEXT\r\n";

    http_parse(httpStr, 0, NULL);

    return 0;
}
