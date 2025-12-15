#ifndef TCP_SERVER_CLIENT_H
#define TCP_SERVER_CLIENT_H

#include <stdbool.h>

#include "../shared/tcp_shared.h" /* TODO: SS - We should not have to go back here. Figure out why. Probably a problem with the Makefile. */

#ifndef TCP_MAX_CLIENT_BUFFER_SIZE
    #define TCP_MAX_CLIENT_BUFFER_SIZE 1024
#endif

typedef struct 
{
    uint32_t unique_id;
    Socket socket;
    bool in_use;    /* TODO: HW - Use states enum */
    bool close_connection;
    uint64_t timestamp;

    uint8_t receive_buffer[TCP_MAX_CLIENT_BUFFER_SIZE];
    uint8_t outgoing_buffer[TCP_MAX_CLIENT_BUFFER_SIZE];
    uint32_t outgoing_buffer_amount_of_bytes;

} TCP_Server_Client;

void tcp_server_client_init(TCP_Server_Client* client);

void tcp_server_client_dispose(TCP_Server_Client* client);

/* Returns: true if client should be timedout else returns false */
bool tcp_server_client_should_timeout(TCP_Server_Client* client);

bool tcp_server_client_get_accepted(TCP_Server_Client* client, int cfd);

bool tcp_server_client_send(TCP_Server_Client* client);

bool tcp_server_client_should_close(TCP_Server_Client* client);

#endif