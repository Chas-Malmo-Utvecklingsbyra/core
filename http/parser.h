#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stdio.h>

typedef struct
{
    char* key;
    char* value;
} Http_Request_Line;

typedef enum
{
    POST,
    GET,
    OPTIONS
} Http_Method;

#ifndef HTTP_REQUEST_PATH_MAX_LENGTH
#define HTTP_REQUEST_PATH_MAX_LENGTH 512
#endif

#ifndef HTTP_REQUEST_QUERY_MAX_LENGTH
#define HTTP_REQUEST_QUERY_MAX_LENGTH 512
#endif

typedef struct
{
    Http_Method method;
    char path[HTTP_REQUEST_PATH_MAX_LENGTH];
    char query[HTTP_REQUEST_QUERY_MAX_LENGTH];
    char http[9];
} Http_Request_Start_Line;

typedef struct
{
    Http_Request_Start_Line start_line;
    Http_Request_Line* request_lines;
    size_t filled_to;
    size_t size;
    char* data;
} Http_Request;


Http_Request* Http_Parser_Parse(const char* buffer);
void Http_Parser_Cleanup(Http_Request* request);
char* Http_Request_Get_Value_From_Key(const Http_Request* request, const char* key);
char* Http_Request_Get_Method_String(const Http_Request* request);

#endif