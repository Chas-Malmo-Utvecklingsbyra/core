#include "parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../string/strdup.h"

/*

#ifndef HTTP_PARSER_DEBUG
    #define HTTP_PARSER_DEBUG
#endif

*/

void Http_Parser_Cleanup(Http_Request** requestPTR)
{
    Http_Request* request = *requestPTR;

    for (size_t i = 0; i < request->filled_to; i++)
    {
        if (request->request_lines == NULL)
            break;

        if (request->request_lines[i].key == NULL || request->request_lines[i].value == NULL)
            break;

        free(request->request_lines[i].key);
        free(request->request_lines[i].value);
    }

    if (request->request_lines)
    {
        free(request->request_lines);
        request->request_lines = NULL;
    }

    if (request->data)
    {
        free(request->data);
    }

    if (requestPTR && *requestPTR)
    {
        free(*requestPTR);
        *requestPTR = NULL;
    }
}

char* Http_Request_Get_Value_From_Key(const Http_Request* request, const char* key)
{
    for (size_t i = request->filled_to-1; i > 0; i--)
    {
        if (request->request_lines[i].key == NULL)
            continue;

        if (strcmp(request->request_lines[i].key, key) == 0)
        {
            return request->request_lines[i].value;
        }
    }

    return NULL;
}

void Http_Request_Line_Add(Http_Request* request, Http_Request_Line line)
{
    if (request == NULL)
    {
        printf("This is null!\n");
        return;
    }

    if (request->request_lines == NULL)
    {
        request->size = 4;
        request->filled_to = 1;
        request->request_lines = (Http_Request_Line*)malloc(sizeof(Http_Request_Line)*4);

        request->request_lines[0] = line;

        if (request->request_lines == NULL)
        {
            printf("MALLOC FAILED AT %s\n", __FILE__);
            return;
        }
    }
    else
    {

        if (request->size == request->filled_to)
        {
            request->size += 4;
            request->request_lines = (Http_Request_Line*)realloc(request->request_lines, sizeof(Http_Request_Line)*request->size);

            if (request->request_lines == NULL)
            {
                printf("REALLOC FAILED AT %s\n", __FILE__);
                return;
            }

        }

        request->filled_to++;
        request->request_lines[request->filled_to-1] = line;
    }
}

void Http_Request_Line_Print(Http_Request_Line* line)
{
    printf("Request Line: [%s] [%s]\n", line->key, line->value);
}

void Http_Request_Print_All(Http_Request* request)
{
    printf("\n\n");
    for (size_t i = 0; i < request->filled_to; i++)
    {
        Http_Request_Line_Print(&request->request_lines[i]);
    }
}

Http_Request* Http_Parser_Parse(const char* buffer)
{
    printf("Buffer to parse: %s\n", buffer);

    if (buffer == NULL)
        return NULL;

    Http_Request* request = (Http_Request*)malloc(sizeof(Http_Request));
    memset(request, 0, sizeof(Http_Request));

    if (request == NULL)
        return NULL;

    char* duped = strdup(buffer);
    char* line = duped;
    bool is_first_line = true;
    bool has_found_body = false;

    while (line != NULL)
    {
        char* line_start = line;

        char* end = strstr(line, "\r\n");
        if (!end)
        {
            break;
        }

        *end = '\0';
        line = end + 2;


        /* Adds to the Http_Request_Line list */
        if (strlen(line_start) > 0 && !is_first_line)
        {


            Http_Request_Line request_line = {0};

            size_t line_size = strlen(line_start);

            request_line.key = (char*)malloc(line_size);
            request_line.value = (char*)malloc(line_size);

            memset(request_line.key, 0, line_size);
            memset(request_line.value, 0, line_size);

            sscanf(line_start, "%[^:]: %[^\r\n]", request_line.key, request_line.value);
            printf("[%s][%s]\n", request_line.key, request_line.value);
            Http_Request_Line_Add(request, request_line);


#ifdef HTTP_PARSER_DEBUG
            printf("Key, Value: [%s][%s]\n", request_line.key, request_line.value);
#endif
        }

        if (is_first_line)
        {
            char method[8] = {0};
            sscanf(line_start, "%7s %399s %8s", method, request->start_line.path, request->start_line.http);

            if (strcmp(method, "POST") == 0)
            {
                request->start_line.method = POST;
            }
            else if (strcmp(method, "GET") == 0)
            {
                request->start_line.method = GET;
            }
            else if (strcmp(method, "OPTIONS") == 0)
            {
                request->start_line.method = OPTIONS;
            }
            else
            {
                printf("Unsupported HTTP method\n");
                free(duped);
                duped = NULL;
                return NULL; /*  */
            }


            is_first_line = false;
        }

        free(duped);
        duped = NULL;
        if (line_start[0] == '\0') {
            has_found_body = true;
            break;
        }
        is_first_line = false;
    }

    if (has_found_body && request->start_line.method == POST)
    {
        char* header_value = Http_Request_Get_Value_From_Key(request, "Content-Length");

        /* Could not find Content-Length thus making data not possible to get without risking security as of right now */
        if (header_value == NULL)
            return NULL;

        char* pend;
        long int header_value_as_int = strtol(header_value, &pend, 10);

        /* Converting the number was not possible */
        if (header_value_as_int == 0L)
        {
            return NULL;
        }

        request->data = malloc(header_value_as_int+1);
        request->data[header_value_as_int] = '\0';
        strncpy(request->data, line, header_value_as_int);
    }

#ifdef HTTP_PARSER_DEBUG
    printf("Body: [%s]\n", request->data);
#endif

    return request;
}

char* Http_Request_Get_Method_String(const Http_Request* request)
{
    switch (request->start_line.method)
    {
        case POST:
            return "POST";

        case GET:
            return "GET";

        case OPTIONS:
            return "OPTIONS";

        default:
            return "Unsupported Method";
    }
}