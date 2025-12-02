#ifndef __HTTP_H__
#define __HTTP_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HTTP_Method_GET,
    HTTP_Method_POST,
    HTTP_Method_PUT,
} HTTP_Method;

bool http_create_response(uint8_t *buffer, uint32_t buffer_length, char *body, uint32_t body_length, uint32_t *out_bytes_written_to_buffer);


#endif /* __HTTP_H__ */