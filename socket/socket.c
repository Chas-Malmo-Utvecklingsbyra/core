#ifdef _WIN32
    #include <winsock2.h>
    #include <w2tcpip.h>
    static bool winsock_started = false;
#else
    #define _POSIX_C_SOURCE 200809L
    #include <arpa/inet.h>
    #include <sys/fcntl.h>
    #include <unistd.h>
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "socket.h"
#include "../utils/clock_monotonic.h"

/* TODO: SS - Support different operating systems (and architectures(?)). */


/* clock monotonic moved to utils directory */



/* Tries to bind the socket to the port. */
Socket_Result socket_open_client(const uint32_t port, const char* ip_string, Socket *out_socket) {

    socket_fd_t fd =
#ifdef _WIN32
    socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    socket(AF_INET, SOCK_STREAM, 0);
#endif

    if (fd < 0) {
        perror("Socket creation error");
        return Socket_Result_Port_Already_Used;
    }

    out_socket->file_descriptor = (uint32_t)fd;

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    /* 2. Konvertera IP-adress */
    if (inet_pton(AF_INET, ip_string, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        return Socket_Result_Invalid_Address;
    }

    /* 3. Anslut till servern */
    if (connect(out_socket->file_descriptor, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
#ifdef _WIN32
        if(WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != WSAEINPROGRESS){
            perror("Connection Failed");
            return Socket_Result_Connection_Failed; 
        }
#else   
        perror("Connection Failed");
        return Socket_Result_Connection_Failed;
#endif
    }

    return Socket_Result_OK;
}

/* Tries to close the socket. */
Socket_Result socket_close(Socket *socket){
#ifdef _WIN32
    closesocket(socket->file_descriptor);
#else
    close(socket->file_descriptor);
#endif
    socket->file_descriptor = -1;
    /* free(socket); */
    return Socket_Result_OK;
}

/* Tries to read (at most) 'buffer_size' bytes from the socket's 'file_descriptor' adds them to 'buffer'. remove the timeout, zero friction implementation please!*/
Socket_Result socket_read(Socket *socket, uint8_t *buffer, const uint32_t buffer_size, int* out_TotalBytesRead) {
    
    *out_TotalBytesRead = 0;

#ifdef _WIN32
    int bytesRead = recv(socket->file_descriptor, buffer, buffer_size, 0);
    if(bytesRead < 0){
        int err = WSAGetLastError();
        if(err == WSAEWOULDBLOCK){
            return Socket_Result_Nothing_Read;
        }
        return socket_result_error;
    }
#else
    int bytesRead = recv(socket->file_descriptor, buffer, buffer_size, MSG_DONTWAIT);
    if(bytesRead < 0)
    {
        if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return Socket_Result_Nothing_Read;
        }         
        return socket_result_error;
    }
#endif         

    if(bytesRead == 0){
        return socket_result_connection_closed;
    }

    *out_TotalBytesRead = bytesRead;

    return Socket_Result_OK;
}

/* Tries to write 'buffer_size' bytes from 'buffer' to the socket's 'file_descriptor'. */
Socket_Result socket_write(Socket *socket, const uint8_t *buffer, const uint32_t buffer_size, uint32_t *out_bytes_sent) {

    assert(socket != NULL);
    assert(buffer != NULL);
    assert(buffer_size > 0);
    assert(out_bytes_sent != NULL);

    int bytesSent = 0;

#ifdef _WIN32
    bytesSent = send(socket->file_descriptor, &buffer[0], buffer_size, 0);
    if(bytesSent < 0)
    {
        int err = WSAGetLastError();
        if(err == WSAEWOULDBLOCK){
            *out_bytes_sent = 0;
            return Socket_Result_Nothing_Written_Yet;
        } else {
            *out_bytes_sent = 0;
            return socket_result_error_socket_write;
        }
    }
#else
    bytesSent = send(socket->file_descriptor, &buffer[0], buffer_size, MSG_NOSIGNAL | MSG_DONTWAIT);
    if(bytesSent < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            *out_bytes_sent = 0;
            return Socket_Result_Nothing_Written_Yet;
        } else {
            *out_bytes_sent = 0;
            return socket_result_error_socket_write;
        }
    }
#endif
    
    /* TODO: (PR) verifiera att buffer ej är NULL
    verifiera att buffer size är större än 0
    verifiera att socket inte ät NULL
    */
    if (bytesSent == 0){
        *out_bytes_sent = 0;
        return socket_result_connection_closed;
    }

    *out_bytes_sent = (uint32_t)bytesSent;
    
    return Socket_Result_OK;
}

