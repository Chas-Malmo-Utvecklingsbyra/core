
#include "http.h"
#include <string.h>
#include <stdio.h>

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
 * @brief Converts an Http_Content_Type enum to its corresponding string representation.
 * @param content_type The Http_Content_Type enum value.
 * @return char* The string representation of the content type.
 */
char* http_get_content_type_string(int content_type)
{
    switch((Http_Content_Type)content_type)
    {
        case HTTP_CONTENT_TYPE_HTML:
            return "text/html; charset=UTF-8";
        case HTTP_CONTENT_TYPE_JSON:
            return "application/json; charset=UTF-8";
        default:
            return "text/plain; charset=UTF-8";
    }
}

char *Http_Get_Method_String(Http_Method method)
{
    switch ((Http_Method)method)
    {
        case HTTP_METHOD_GET:
            return "GET";
            break;
        case HTTP_METHOD_POST:
            return "POST";
            break;
        case HTTP_METHOD_PUT:
            return "PUT";
            break;
        case HTTP_METHOD_DELETE:
            return "DELETE";
            break;
        case HTTP_METHOD_UNDEFINED:
            return "UNDEFINED";
            break;
        
        default:
            break;
    };
    return "UNDEFINED";
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
bool http_create_response(uint8_t *buffer, uint32_t buffer_length, const char *body, uint32_t body_length, const HTTP_Status_Code http_status_code, uint32_t *out_bytes_written_to_buffer, Http_Content_Type content_type){
    /* TODO: (SS) Create our own sNprintf() func/lib */

    *out_bytes_written_to_buffer = snprintf((char*)buffer, buffer_length, "HTTP/1.1 %s\r\n"
    "Date: Mon, 10 Nov 2025 15:40:00 GMT\r\n" /* TODO: HW - Use actual time */
    "Server: Apache/2.4.41 (Ubuntu)\r\n" /* TODO: HW - Use actual server name/type */
    "Content-Type: %s\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
    "Access-Control-Allow-Headers: Content-Type, Content-Length\r\n"
    "Content-Length: %d\r\n"
    "Connection: close\r\n"
    "\r\n"
    "%s"
    ,http_get_status_code_string(http_status_code)
    ,http_get_content_type_string(content_type), body_length, body);

    return true;
}