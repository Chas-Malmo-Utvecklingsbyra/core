#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

/* (PR) The stuff we created last wednesday(?), start of basic http parser */


int http_parse(const char* _InBuffer, size_t _InBufferSize, char* _OutBuffer)
{
    char* dataCopy = strdup(_InBuffer);
    char* ptr = dataCopy;
    char* headers = NULL;
    char* method = NULL;
    char* path = NULL;
    char* version = NULL;

    /*
    EXAMPLE OF HTTP TEXT:

    GET /weather?city=Stockholm HTTP/1.1\r\n
    Host: 127.0.0.1\r\n
    User-Agent: curl/8.0\r\n
    \r\n


    TO HELP MAKE PARSER LOGIC.
    */

    do{

        if (method == NULL)
        {
            method = ptr;
        }
        else if (path == NULL)
        {
            if(*(ptr) == ' ')
            {
                path = ptr +1;
                *(ptr) = '\0';
            }
        }
        else if (version == NULL)
        {
            if(*(ptr) == ' ')
            {
                version = ptr +1;
                *(ptr) = '\0';
                
            }

        }
        else if(headers == NULL)
        {
            if(*(ptr) == '\n')
            {
                headers = ptr +1;
                *(ptr) = '\0';
                break;
            }
        }

        ptr++;

    }while (*(ptr) != '\0');

    printf("Method : %s\n", method);
    printf("Path   : %s\n", path);
    printf("Version: %s\n", version);

    printf("Headers: %s\n", headers);

    return 0;
}

int http_parseBody();

int http_parseHeader();

/*
int main(){

    char* httpStr = "GET /weather?city=Stockholm HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/8.0\r\n\r\n";

    http_parse(httpStr, 0, NULL);

    return 0;
}
*/