#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>
// TODO: SS - Support different operating systems (and architectures(?)).

typedef struct {
    uint32_t file_descriptor;
} Socket;

typedef enum {
    Socket_Result_OK,
    Socket_Result_Port_Already_Used,
    Socket_Result_Invalid_Address,
    Socket_Result_Connection_Failed,

    // ..
} Socket_Result;

// Tries to bind the socket to the port.
Socket_Result socket_open(const uint32_t port, Socket *out_socket);

// Tries to close the socket.
Socket_Result socket_close(Socket *socket);

// Tries to read (at most) 'buffer_size' bytes from the socket's 'file_descriptor' adds them to 'buffer'.
Socket_Result socket_read(Socket *socket, uint8_t *buffer, const uint32_t buffer_size);

// Tries to write 'buffer_size' bytes from 'buffer' to the socket's 'file_descriptor'.
Socket_Result socket_write(Socket *socket, const uint8_t *buffer, const uint32_t buffer_size);

#endif