#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>
/* TODO: SS - Support different operating systems (and architectures(?)). */

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_fd_t;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_fd_t;
#endif 

typedef struct {
    uint32_t file_descriptor;
} Socket;

typedef enum {
    Socket_Result_OK,
    Socket_Result_Port_Already_Used,
    Socket_Result_Invalid_Address,
    Socket_Result_Connection_Failed,
    Socket_Result_Nothing_Read,
    Socket_Result_OK_Something_Read,
    Socket_Result_Nothing_Written_Yet,
    Socket_Result_OK_Write_Success,
    Socket_Result_Fail_Write_Timeout,
    socket_result_connection_closed,
    socket_result_error,
    socket_result_error_socket_write,

    /* ... */
} Socket_Result;

bool socket_global_init(void);

void socket_global_cleanup(void);

/* Tries to bind the socket to the port. */
Socket_Result socket_open(const uint32_t port, Socket *out_socket);

/* Tries to close the socket. */
Socket_Result socket_close(Socket *socket);

/* Tries to read (at most) 'buffer_size' bytes from the socket's 'file_descriptor' adds them to 'buffer'. */
Socket_Result socket_read(Socket *socket, uint8_t *buffer, const uint32_t buffer_size, int* out_TotalBytesRead);

/* Tries to write 'buffer_size' bytes from 'buffer' to the socket's 'file_descriptor'. */
Socket_Result socket_write(Socket *socket, const uint8_t *buffer, const uint32_t buffer_size, uint32_t *out_bytes_sent);

#endif