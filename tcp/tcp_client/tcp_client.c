#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#endif

#include "tcp_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../../utils/min.h"

/* Private functions */

static TCP_Client_Result tcp_client_read(TCP_Client *client);
static TCP_Client_Result tcp_client_send_queued(TCP_Client *client);
static TCP_Client_Result tcp_client_requested_disconnect(TCP_Client *client);

/*-------------------*/

TCP_Client_Result tcp_client_init(TCP_Client *client, void* _Context,
                                TCP_Client_Callback_On_Received_Bytes_From_Server on_received, 
                                TCP_Client_Callback_On_Connect on_connect, 
                                TCP_Client_Callback_On_Disconnect on_disconnect, 
                                TCP_Client_Callback_On_Error on_error){
    
    assert(client != NULL);
    assert(on_received != NULL);
    assert(on_connect != NULL);
    assert(on_disconnect != NULL);
    assert(on_error != NULL);
    
    assert(client->server.connection_state == TCP_Client_Connection_State_Disconnected);
    
    /* 
    if(client->connection_state == TCP_Client_Connection_State_Initialized){
        return TCP_Client_Result_Already_Initialized;
    } */

    memset(client, 0, sizeof(TCP_Client));   
    
    client->server.socket.file_descriptor = -1;
    client->server.incoming_buffer[0] = 0;
    client->server.outgoing_buffer[0] = 0;
    client->server.outgoing_buffer_bytes = 0;
    client->server.close_requested = false;
    
    client->on_received_callback = on_received;
    client->on_connect_callback = on_connect;
    client->on_disconnect_callback = on_disconnect;
    client->on_error_callback = on_error;
    client->context = _Context;
    
    client->server.connection_state = TCP_Client_Connection_State_Connecting;
    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_connect(TCP_Client *client, const char *ip, int port){

    assert(client != NULL);
    assert(ip != NULL);
    assert(client->server.connection_state == TCP_Client_Connection_State_Connecting || client->server.connection_state == TCP_Client_Connection_State_Disconnected);
    
    if(client->server.connection_state == TCP_Client_Connection_State_Connected){
        return TCP_Client_Result_Already_Connected;
    }    
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);

    struct addrinfo hints;
    struct addrinfo *responses = NULL;
    char port_str[6];
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(port_str, sizeof(port_str), "%d", port);

    err = getaddrinfo(ip, port_str, &hints, &responses);
    if(err != 0 || responses == NULL){
        if(client->on_error_callback){
            client->on_error_callback(client, TCP_Client_Result_Error_Invalid_Address);
        }
        return TCP_Client_Result_Error_Invalid_Address;
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)responses->ai_addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ipv4->sin_port;
    addr.sin_addr = ipv4->sin_addr;

    freeaddrinfo(responses);


    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        if(client->on_error_callback){
            client->on_error_callback(client, TCP_Client_Result_Error_Creating_Socket);
        }
        return TCP_Client_Result_Error_Creating_Socket;
    }

    int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0){
        close(fd);
        if(client->on_error_callback){
            client->on_error_callback(client, TCP_Client_Result_Error_Fcntl);
        }
		return TCP_Client_Result_Error_Fcntl;
    }
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    int res = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if(res < 0){ 
        if(errno != EINPROGRESS){
            close(fd);
            if(client->on_error_callback){
                client->on_error_callback(client, TCP_Client_Result_Error_Connection_Failure);
            }
            return TCP_Client_Result_Error_Connection_Failure;
        }
    }

    client->server.socket.file_descriptor = (uint32_t)fd;
    client->server.connection_state = TCP_Client_Connection_State_Connected;
    if(client->on_connect_callback) client->on_connect_callback(client);
    client->server.outgoing_buffer_bytes = client->server.outgoing_buffer_bytes;
    
    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_read(TCP_Client *client){
    
    assert(client != NULL);
     
    if(client->server.connection_state != TCP_Client_Connection_State_Connected && client->server.connection_state != TCP_Client_Connection_State_Working){
        return TCP_Client_Result_Disconnected;
    }

    int totalBytesRead = 0;

    Socket_Result result = socket_read(&client->server.socket, client->server.incoming_buffer, TCP_CLIENT_RECEIVE_BUFFER_SIZE, &totalBytesRead);

    if(result == socket_result_connection_closed){
        client->server.connection_state = TCP_Client_Connection_State_Disconnected;
        if(client->on_disconnect_callback){
            client->on_disconnect_callback(client);
        }
        return TCP_Client_Result_Disconnected;
    }

    if(result != Socket_Result_OK){
        return TCP_Client_Result_OK;
    }

    if(totalBytesRead > 0 && client->on_received_callback){
        client->on_received_callback(client, client->server.incoming_buffer, (uint32_t)totalBytesRead);
    }

    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_send_queued(TCP_Client *client){

    assert(client != NULL);
    
    if(client->server.connection_state != TCP_Client_Connection_State_Connected && client->server.connection_state != TCP_Client_Connection_State_Working){
        return TCP_Client_Result_Disconnected;
    }

    if(client->server.outgoing_buffer_bytes == 0){
        return TCP_Client_Result_OK;
    }

    uint32_t sent = 0;

    Socket_Result write_result = socket_write(&client->server.socket, client->server.outgoing_buffer, client->server.outgoing_buffer_bytes, &sent);

    if(write_result != Socket_Result_OK){
        if(write_result == socket_result_connection_closed){
            client->server.connection_state = TCP_Client_Connection_State_Disconnected;
            client->on_disconnect_callback(client);
            return TCP_Client_Result_Disconnected;
        }
        return TCP_Client_Result_OK;
    }

    if(sent > 0){
        if(sent < client->server.outgoing_buffer_bytes){
            uint32_t remaining = client->server.outgoing_buffer_bytes - sent;
            memmove(client->server.outgoing_buffer, client->server.outgoing_buffer + sent, remaining);
            client->server.outgoing_buffer_bytes = remaining;
        } else {
            client->server.outgoing_buffer_bytes = 0;
        }
    }

    return TCP_Client_Result_OK;
}


TCP_Client_Result tcp_client_work(TCP_Client *client){

    assert(client != NULL);
    assert(client->server.connection_state != TCP_Client_Connection_State_Disconnected);

    if(client->server.close_requested){
        return tcp_client_requested_disconnect(client);
    }

    if(client->working){
        if(client->on_error_callback){
            client->on_error_callback(client, TCP_Client_Result_Error_Trying_To_Work_Again);
        }
        return TCP_Client_Result_Already_Working;
    }

    client->working = true;
    client->server.connection_state = TCP_Client_Connection_State_Working;

    TCP_Client_Result reading = tcp_client_read(client);
    if(reading == TCP_Client_Result_Disconnected){
        client->server.connection_state = TCP_Client_Connection_State_Disconnected;
        client->working = false;
        return TCP_Client_Result_Disconnected;
    }

    if(reading != TCP_Client_Result_OK){
        client->server.connection_state = TCP_Client_Connection_State_Stopped_Working;
        client->working = false;
        return reading;
    }

    TCP_Client_Result send_queue = tcp_client_send_queued(client);
    if(send_queue != TCP_Client_Result_OK){
        client->server.connection_state = TCP_Client_Connection_State_Stopped_Working;
        client->working = false;
        return TCP_Client_Result_Error_Sending_Queued;
    }

    if(client->server.connection_state == TCP_Client_Connection_State_Stopped_Working){
        client->server.connection_state = TCP_Client_Connection_State_Connected;
    }

    client->working = false;
    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_send(TCP_Client *client, const uint8_t *buffer, uint32_t size){

    assert(client != NULL);
    assert(buffer != NULL);

    if(client->server.connection_state != TCP_Client_Connection_State_Connected && client->server.connection_state != TCP_Client_Connection_State_Working){
        return TCP_Client_Result_Disconnected;
    }
    
    uint32_t space_left = TCP_CLIENT_OUTGOING_BUFFER_SIZE - client->server.outgoing_buffer_bytes;

    if(space_left < size){
        return TCP_Client_Result_Not_Enough_Space;
    }

    uint32_t copy = min_uint32(space_left, size);
    memcpy(&client->server.outgoing_buffer[client->server.outgoing_buffer_bytes], buffer, copy);
    client->server.outgoing_buffer_bytes += copy;

    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_disconnect(TCP_Client *client){

    assert(client != NULL);

    if(client->server.socket.file_descriptor > 0){
        socket_close(&client->server.socket);
    }

    client->server.connection_state = TCP_Client_Connection_State_Disconnected;
    client->server.outgoing_buffer_bytes = 0;
    client->on_disconnect_callback(client);

    return TCP_Client_Result_OK;    
}

TCP_Client_Result tcp_client_requested_disconnect(TCP_Client *client){
    assert(client != NULL);

    client->server.close_requested = true;
    return TCP_Client_Result_OK;
}

