#include "parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../string/strdup.h"

#ifndef HTTP_PARSER_DEBUG
    #define HTTP_PARSER_DEBUG
#endif

void Http_Parser_Cleanup(Http_Request* request)
{
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
}

char* Http_Request_Get_Value_From_Key(Http_Request* request, const char* key)
{
    for (int i = request->filled_to-1; i > 0; i--)
    {
        if (strcmp(request->request_lines[i].key, key) == 0)
        {
            return request->request_lines[i].value;
        }
    }

    return NULL;
}

void Http_Request_Line_Add(Http_Request* request, Http_Request_Line line)
{
    if (request->request_lines == NULL)
    {
        request->size = 4;
        request->filled_to = 1;
        request->request_lines = (Http_Request_Line*)malloc(sizeof(Http_Request_Line)*4);
        request->request_lines[0] = line;
    }
    else
    {

        if (request->size == request->filled_to)
        {
            request->size += 4;
            request->request_lines = (Http_Request_Line*)realloc(request->request_lines, sizeof(Http_Request_Line)*request->size);
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
    if (buffer == NULL) 
        return NULL;

    Http_Request* new_request = (Http_Request*)malloc(sizeof(Http_Request));

    if (new_request == NULL)
        return NULL;

    memset(new_request, 0, sizeof(Http_Request));

    
    char line[1024];
    char key[256];
    char value[1024];

    char* copy = strdup(buffer);
    char* ptr = copy;

    bool first_line = true;
    bool found_json = false;
    char* data_buffer = NULL;

    while (*ptr)
    {
        char* line_end = strchr(ptr, '\n');
        if (line_end)
        {
            size_t len = line_end - ptr;
            if (len >= sizeof(line)) len = sizeof(line) - 1;
            strncpy(line, ptr, len);
            line[len] = '\0';
            ptr = line_end + 1;
        }
        else
        {
            strncpy(line, ptr, sizeof(line) - 1);
            line[sizeof(line) - 1] = '\0';
            ptr += strlen(ptr);
        }

        /* trim /r */
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len - 1] == '\r')
            line[line_len - 1] = '\0';

        if (first_line)
        {
            first_line = false;
            int sscanf_return = sscanf(line, "%s %s %s",
                                       new_request->start_line.method,
                                       new_request->start_line.path,
                                       new_request->start_line.http);
            if (sscanf_return != 3)
                return NULL;

            continue;
        }

        memset(key, 0, sizeof(key));
        memset(value, 0, sizeof(value));
        sscanf(line, "%[^:]: %[^\r\n]", key, value);

        if (strlen(key) > 0)
        {
            Http_Request_Line http_line = {0};
            http_line.key = strdup(key);
            http_line.value = strdup(value);
            Http_Request_Line_Add(new_request, http_line);

            if (strcmp(key, "Content-Length") == 0 && !found_json && strcmp(new_request->start_line.method, "POST") == 0)
            {
                int content_length_num = atoi(value);
                if (content_length_num <= 0)
                    return NULL;

                data_buffer = (char*)malloc(content_length_num+1);
                memset(data_buffer, 0, content_length_num+1);

                strncpy(data_buffer, ptr+2, content_length_num);

                new_request->data = data_buffer;
                found_json = true;
            }
        }
    }
    free(copy);
    return new_request;
}
