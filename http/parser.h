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
}Http_Method;

typedef struct
{
    Http_Method method;
    char path[400];
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
void Http_Parser_Cleanup(Http_Request** requestPTR);
char* Http_Request_Get_Value_From_Key(const Http_Request* request, const char* key);
char* Http_Request_Get_Method_String(const Http_Request* request);

#endif