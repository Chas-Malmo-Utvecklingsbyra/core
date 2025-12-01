
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

bool http_create_response(uint8_t *buffer, uint32_t buffer_length, char *body, uint32_t body_length, uint32_t *out_bytes_written_to_buffer){
    (void)buffer_length;
    /* TODO: (SS) Create our own sNprintf() func/lib */
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


    
    return true;
}

bool http_create_CORS_response(uint8_t *buffer, uint32_t buffer_length, char *body, uint32_t body_length, uint32_t *out_bytes_written_to_buffer){
    (void)buffer_length;
    (void)body;
    (void)body_length;
    /* TODO: (SS) Create our own sNprintf() func/lib */
    *out_bytes_written_to_buffer = sprintf((char*)buffer, "HTTP/1.1 200 OK\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                    "Access-Control-Allow-Headers: Content-Type, Content-Length\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    );
/* "Access-Control-Allow-Headers:Content-Type,Authorization\r\n" USE YES OR NO? */
/*                     "Date: Mon, 10 Nov 2025 15:40:00 GMT\r\n"
                    "Server: Apache/2.4.41 (Ubuntu)\r\n" */

    
    return true;
}