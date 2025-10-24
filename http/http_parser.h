
#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__
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


int http_parse(const char* _InBuffer, size_t _InBufferSize, HttpRequestBlob* _HttpBlob);

int http_destroyBlob(HttpRequestBlob* _Blob);

/* NYI */
int http_parseBody();

/* NYI */
int http_parseHeader();


#endif /* __HTTP_PARSER_H__ */