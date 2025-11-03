#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "../shared/tcp_shared.h" // TODO: SS - We should not have to go back here. Figure out why. Probably a problem with the Makefile.

#ifndef TCP_MAX_CLIENTS_PER_SERVER
// NOTE: SS - Slightly annoying because this means that all servers have to have max N clients if we'd have multiple servers per program for example.
// It would be nice if we could specify this on a per-server basis.
#define TCP_MAX_CLIENTS_PER_SERVER 64
#endif

typedef struct TCP_Server TCP_Server;
typedef struct TCP_Server_Client TCP_Server_Client;

typedef void (*TCP_Server_Callback_On_Recieved_Bytes_From_Client)(const TCP_Server *server, const TCP_Server_Client *client, const uint8_t *buffer, const uint32_t buffer_size);

struct TCP_Server_Client {
    uint32_t unique_id;
    Socket socket;

    // NOTE: SS - We could have two buffers here; one for input and one for output. 'receive_buffer'.
};

struct TCP_Server {
    Socket socket;
    int port;

    TCP_Server_Client clients[TCP_MAX_CLIENTS_PER_SERVER];
    uint32_t client_count;

    uint32_t next_unique_id;

    TCP_Server_Callback_On_Recieved_Bytes_From_Client on_received_bytes_from_client;
};

typedef enum {
    TCP_Server_Result_OK,
    TCP_Server_Result_Port_In_Use,
    // ..
} TCP_Server_Result;

TCP_Server_Result tcp_server_init(TCP_Server *server, TCP_Server_Callback_On_Recieved_Bytes_From_Client on_received_bytes_from_client);

TCP_Server_Result tcp_server_start(TCP_Server *server);
TCP_Server_Result tcp_server_stop(TCP_Server *server);

TCP_Server_Result tcp_server_dispose(TCP_Server *server);

#endif