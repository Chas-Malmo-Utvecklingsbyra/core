
#include "http.h"
#include <string.h>
#include <stdio.h>

/* 
"HTTP/1.1 200 OK\r\n"
"Date: Mon, 10 Nov 2025 15:40:00 GMT\r\n"
"Server: Apache/2.4.41 (Ubuntu)\r\n"
"Content-Type: text/html; charset=UTF-8\r\n"
"Content-Length: %d\r\n"
"Connection: close\r\n\r\n"
"<html>"
"<head><title>Welcome</title></head>"
"<body><h1>Hello, world!</h1></body>"
"</html>"
"\r\n"
 */

/**
 * @brief Converts an HTTP status code integer to its corresponding string representation.
 * @param http_status_code The HTTP status code as an integer.
 * @return char* The string representation of the HTTP status code.
 */
char* http_get_status_code_string(int http_status_code)
{
    switch((HTTP_Status_Code)http_status_code)
    {
        case HTTP_STATUS_CODE_OK:
            return "200 OK";
        case HTTP_STATUS_CODE_BAD_REQUEST:
            return "400 Bad Request";
        case HTTP_STATUS_CODE_NOT_FOUND:
            return "404 Not Found";
        case HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR:
            return "500 Internal Server Error";
        default:
            return "500 Internal Server Error";
    }
}

/**
 * @brief Creates an HTTP response and writes it to the provided buffer.
 * @param buffer Pointer to the buffer where the response will be written.
 * @param buffer_length Length of the provided buffer.
 * @param http_status_code HTTP status code for the response.
 * @param body Pointer to the response body.
 * @param body_length Length of the response body.
 * @param out_bytes_written_to_buffer Pointer to store the number of bytes written to the buffer
 * @return true if the response was created successfully
 */
bool http_create_response(uint8_t *buffer, uint32_t buffer_length, char *body, uint32_t body_length, uint32_t *out_bytes_written_to_buffer, Http_Content_Type content_type){
    (void)buffer_length;
    /* TODO: (SS) Create our own sNprintf() func/lib */
    
    if (content_type == HTTP_CONTENT_TYPE_JSON)
    {
        *out_bytes_written_to_buffer = sprintf((char*)buffer, "HTTP/1.1 200 OK\r\n"
                        "Date: Mon, 10 Nov 2025 15:40:00 GMT\r\n"
                        "Server: Apache/2.4.41 (Ubuntu)\r\n"
                        "Content-Type: application/json; charset=UTF-8\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                        "Access-Control-Allow-Headers: Content-Type, Content-Length\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "%s"
                        ,body_length, body);
    }
    else
    {
        *out_bytes_written_to_buffer = sprintf((char*)buffer, "HTTP/1.1 200 OK\r\n"
                        "Date: Mon, 10 Nov 2025 15:40:00 GMT\r\n"
                        "Server: Apache/2.4.41 (Ubuntu)\r\n"
                        "Content-Type: text/html; charset=UTF-8\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                        "Access-Control-Allow-Headers: Content-Type, Content-Length\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "%s"
                        ,body_length, body);
    }
    
    
    return true;
}