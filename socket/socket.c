#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include "socket.h"
#include <unistd.h>
#include <assert.h>

#include "../utils/clock_monotonic.h"

/* TODO: SS - Support different operating systems (and architectures(?)). */

/* clock monotonic moved to utils directory */



/* Tries to bind the socket to the port. */
Socket_Result socket_open(const uint32_t port, Socket *out_socket) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Socket creation error");
        return Socket_Result_Port_Already_Used;
    }

    out_socket->file_descriptor = (uint32_t)fd;

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    /* 2. Konvertera IP-adress */
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        return Socket_Result_Invalid_Address;
    }

    /* 3. Anslut till servern */
    if (connect(out_socket->file_descriptor, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return Socket_Result_Connection_Failed;
    }

    return Socket_Result_OK;
}

/* Tries to close the socket. */
Socket_Result socket_close(Socket *socket){
    close(socket->file_descriptor);
    /* free(socket); */
    return Socket_Result_OK;
}

/* Tries to read (at most) 'buffer_size' bytes from the socket's 'file_descriptor' adds them to 'buffer'. remove the timeout, zero friction implementation please!*/
Socket_Result socket_read(Socket *socket, uint8_t *buffer, const uint32_t buffer_size, int* out_TotalBytesRead) {
	int totalBytesRead = 0;

    int bytesRead = recv(
        socket->file_descriptor,
        &buffer[totalBytesRead],
        buffer_size - totalBytesRead,
        MSG_DONTWAIT
    );

    if(bytesRead <= 0) {
        return Socket_Result_Nothing_read;
    }

    *(out_TotalBytesRead) = bytesRead;
    return Socket_Result_OK;
}

/* Tries to write 'buffer_size' bytes from 'buffer' to the socket's 'file_descriptor'. */
Socket_Result socket_write(Socket *socket, const uint8_t *buffer, const uint32_t buffer_size, uint32_t *out_bytes_sent) {
    
    *out_bytes_sent = 0;

    int bytesSent = send(socket->file_descriptor, &buffer[0], buffer_size, MSG_NOSIGNAL);
    if(bytesSent > 0)
    {
        *out_bytes_sent = (uint32_t)bytesSent;
        return Socket_Result_OK;
    }

/* TODO: (PR) verifiera att buffer ej är NULL
verifiera att buffer size är större än 0
verifiera att socket inte ät NULL
*/
    if (bytesSent == 0){
        return socket_result_connection_closed;
    }

    return Socket_Result_Fail_Write_Timeout; /* TODO: Replace with better enum */
}

