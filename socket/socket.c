#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <time.h>
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


/* get this from other util lib later */

#define CLOCK_MONOTONIC 1
uint64_t SystemMonotonicMS()
{
	long            ms;
	time_t          s;

	struct timespec spec;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	s  = spec.tv_sec;
	ms = (spec.tv_nsec / 1000000);

	uint64_t result = s;
	result *= 1000;
	result += ms;

	return result;
}

// Tries to bind the socket to the port.
Socket_Result socket_open(const uint32_t port, Socket *out_socket){

    out_socket->file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (out_socket->file_descriptor < 0) {
        perror("Socket creation error");
        return Socket_Result_Port_Already_Used;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // 2. Konvertera IP-adress
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
    return Socket_Result_Invalid_Address;
    }
    // 3. Anslut till servern
    if (connect(out_socket->file_descriptor, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
    return Socket_Result_Connection_Failed;
    }

    return Socket_Result_OK;
}

// Tries to close the socket.
Socket_Result socket_close(Socket *socket){
    close(socket->file_descriptor);
    free(socket);
    return Socket_Result_OK;
}

// Tries to read (at most) 'buffer_size' bytes from the socket's 'file_descriptor' adds them to 'buffer'.
Socket_Result socket_read(Socket *socket, uint8_t *buffer, const uint32_t buffer_size){
    
	int totalBytesRead = 0;
    uint64_t now = SystemMonotonicMS();
    uint64_t timeout = now + 5000; /* 5 seconds timeout */
    while(now < timeout)
		{
			now = SystemMonotonicMS();

            int bytesRead = recv(socket->file_descriptor, buffer[totalBytesRead], buffer_size - totalBytesRead, MSG_DONTWAIT);
			if (bytesRead > 0){
				totalBytesRead += bytesRead;
			}
			
			printf("bytesRead: %d\nTotalBytesRead: %d \n", bytesRead, totalBytesRead);
			

			if(totalBytesRead == buffer_size)
				break;
		}


    printf("Server: %s\n", buffer);

    return Socket_Result_OK;
}

// Tries to write 'buffer_size' bytes from 'buffer' to the socket's 'file_descriptor'.
Socket_Result socket_write(Socket *socket, const uint8_t *buffer, const uint32_t buffer_size){

    const char* ptr = &buffer[0];
    int bytesLeft = buffer_size;

    uint64_t now = SystemMonotonicMS();
    uint64_t timeout = now + 5000;
    
    while(bytesLeft > 0 && now < timeout)
    {
        now = SystemMonotonicMS();
        
        int bytesSent = send(socket->file_descriptor, ptr, bytesLeft, MSG_NOSIGNAL);
        if(bytesSent > 0)
        {
            ptr += bytesSent;
            bytesLeft -= bytesSent;
        }
    }
    if(bytesLeft > 0)
    {
        printf("TIMEOUT ON WRITE!\r\n");
        return 1;
    }




    send(socket->file_descriptor, buffer, buffer_size, MSG_NOSIGNAL);

    return Socket_Result_OK;
}

