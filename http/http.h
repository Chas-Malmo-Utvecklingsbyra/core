#ifndef __HTTP_H__
#define __HTTP_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum HTTP_Status_Code
{
    HTTP_STATUS_CODE_UNKNOWN = 0,
    HTTP_STATUS_CODE_OK = 200,
    HTTP_STATUS_CODE_BAD_REQUEST = 400,
    HTTP_STATUS_CODE_NOT_FOUND = 404,
    HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR = 500

} HTTP_Status_Code;

typedef enum
{
    HTTP_CONTENT_TYPE_HTML,
    HTTP_CONTENT_TYPE_JSON
} Http_Content_Type;

bool http_create_response(uint8_t *buffer, uint32_t buffer_length, char *body, uint32_t body_length, uint32_t *out_bytes_written_to_buffer, Http_Content_Type content_type);


#endif /* __HTTP_H__ */