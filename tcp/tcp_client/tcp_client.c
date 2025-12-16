#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#endif
#include "tcp_client.h"
#include "../../http/parser.h"

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

/*-------------------*/

/* -----------------Helpers------------------- */

void sleep_ms(int milliseconds){
    sleep(milliseconds / 1000); /* Linux takes seconds, Windows expects milliseconds. TODO: PL - Make abstraction (utility) */
}

static int find_headers_end(const uint8_t *buffer, uint32_t buffer_length)
{
    if (buffer == NULL || buffer_length < 4) return -1;
    for (uint32_t i = 0; i + 3 < buffer_length; ++i) {
        if (buffer[i] == '\r' && buffer[i+1] == '\n' && buffer[i+2] == '\r' && buffer[i+3] == '\n') {
            return (int)(i + 4);
        }
    }
    return -1;
}

static int parse_content_length(const uint8_t *buffer, uint32_t buffer_length)
{
    if (!buffer || buffer_length == 0) return -1;
    const char needle[] = "content-length:";
    size_t needle_length = sizeof(needle) - 1;

    for (uint32_t i = 0; i + needle_length < buffer_length; ++i) {
        uint32_t j = 0;
        for (; j < needle_length; ++j) {
            char a = buffer[i + j];
            char b = needle[j];
            if (a >= 'A' && a <= 'Z') a = a - 'A' + 'a';
            if (a != b) break;
        }
        if (j == needle_length) {
            uint32_t p = i + needle_length;
            while (p < buffer_length && (buffer[p] == ' ' || buffer[p] == '\t' || buffer[p] == ':')) p++;
            
            long value = 0;
            bool any = false;
            while (p < buffer_length && buffer[p] >= '0' && buffer[p] <= '9') {
                any = true;
                value = value * 10 + (buffer[p] - '0');
                p++;
                
                if (value > INT32_MAX) break;
            }
            if (any) return (int)value;
            return -1;
        }
    }
    return -1;
}

/* ------------------------------------------- */



TCP_Client_Result tcp_client_work(TCP_Client *client){

    assert(client != NULL);

    if(client->server.close_requested){
        client->server.connection_state = TCP_Client_Connection_State_Disconnecting;
    }

    if(client->working){
        if(client->on_error_callback){
            client->on_error_callback(client, TCP_Client_Result_Error_Trying_To_Work_Again);
        }
        return TCP_Client_Result_Already_Working;
    }

    /* printf("[DEBUG] outgoing bytes waiting: %u\n", client->server.outgoing_buffer_bytes); */

    switch(client->server.connection_state){
        
        case TCP_Client_Connection_State_Disconnected:
            
            return TCP_Client_Result_OK;

        case TCP_Client_Connection_State_Connecting:
        {
            if(client->server.outgoing_buffer_bytes > 0){
                TCP_Client_Result send_result = tcp_client_send_queued(client);
                if(send_result != TCP_Client_Result_OK){
                    if(send_result == TCP_Client_Result_Disconnected){
                        client->server.connection_state = TCP_Client_Connection_State_Disconnected;
                        return TCP_Client_Result_Disconnected;
                    }

                    return send_result;
                }
            }

            {
                int err = 0;
                socklen_t len = sizeof(err);
                int get_sock = getsockopt((int)client->server.socket.file_descriptor, SOL_SOCKET, SO_ERROR, &err, &len);
                if(get_sock < 0){
                    if (client->on_error_callback){
                        client->on_error_callback(client, TCP_Client_Result_Error_Connection_Failure);
                    }
                    client->server.connection_state = TCP_Client_Connection_State_Disconnected;
                    return TCP_Client_Result_Error_Connection_Failure;
                }            

                if(err == 0)
                {
                    client->server.connection_state = TCP_Client_Connection_State_Connected;
                    if(client->on_connect_callback)
                    {
                        client->on_connect_callback(client);
                    }

                    return TCP_Client_Result_OK;
                } else {
#ifdef EINPROGRESS
                    if(err == EINPROGRESS){
                        return TCP_Client_Result_OK;
                    }
#endif
                    client->server.connection_state = TCP_Client_Connection_State_Disconnected;
                    if(client->on_error_callback)
                    {
                        client->on_error_callback(client, TCP_Client_Result_Error_Connection_Failure);
                    }

                    return TCP_Client_Result_Error_Connection_Failure;
                }
            }
        }
        
        case TCP_Client_Connection_State_Connected:
        {
            TCP_Client_Result reading = tcp_client_read(client);
            if(reading == TCP_Client_Result_Disconnected)
            {
                client->server.connection_state = TCP_Client_Connection_State_Disconnected;
                return TCP_Client_Result_Disconnected;
            } else if (reading != TCP_Client_Result_OK && reading != TCP_Client_Result_Nothing_Read_Yet){
                if(client->on_error_callback){
                    client->on_error_callback(client, reading);
                }
                return reading;
            }

            if (!client->working && client->server.outgoing_buffer_bytes > 0) {
                TCP_Client_Result send_result = tcp_client_send_queued(client);

                if (send_result == TCP_Client_Result_Disconnected) {
                    client->server.connection_state = TCP_Client_Connection_State_Disconnected;
                    return TCP_Client_Result_Disconnected;
                }

                if (send_result != TCP_Client_Result_OK && send_result != TCP_Client_Result_Nothing_Sent_Yet) {
                    return send_result;
                }                  
            }

            

            return TCP_Client_Result_OK;
        }
            
        case TCP_Client_Connection_State_Disconnecting:
        {
            socket_close(&client->server.socket);

            client->server.connection_state = TCP_Client_Connection_State_Disconnected;
            client->server.outgoing_buffer_bytes = 0;

            if(client->on_disconnect_callback)
            {
                client->on_disconnect_callback(client);
            }

            return TCP_Client_Result_OK;
        }        
    }

    return TCP_Client_Result_OK;
}



TCP_Client_Result tcp_client_init(TCP_Client *client, void* _Context,
                                TCP_Client_Callback_On_Received_Bytes_From_Server on_received, 
                                TCP_Client_Callback_On_Full_Request on_full_request, 
                                TCP_Client_Callback_On_Connect on_connect, 
                                TCP_Client_Callback_On_Disconnect on_disconnect, 
                                TCP_Client_Callback_On_Error on_error){
    
    assert(client != NULL);
    assert(on_received != NULL);
    assert(on_full_request != NULL);
    assert(on_connect != NULL);
    assert(on_disconnect != NULL);
    assert(on_error != NULL);
    
    assert(client->server.connection_state == TCP_Client_Connection_State_Disconnected);

    memset(client, 0, sizeof(TCP_Client));
    
    client->server.socket.file_descriptor = -1;
    client->server.close_requested = false;
    client->server.incoming_buffer[0] = 0;
    client->server.outgoing_buffer[0] = 0;
    client->server.outgoing_buffer_bytes = 0;
    client->server.incoming_buffer_bytes = 0;
    
    client->on_received_callback = on_received;
    client->on_full_request_callback = on_full_request;
    client->on_connect_callback = on_connect;
    client->on_disconnect_callback = on_disconnect;
    client->on_error_callback = on_error;
    client->context = _Context;

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
    client->server.connection_state = TCP_Client_Connection_State_Connecting;
    client->server.outgoing_buffer_bytes = client->server.outgoing_buffer_bytes;
    
    return TCP_Client_Result_OK;
}


TCP_Client_Result tcp_client_read(TCP_Client *client){
    
    assert(client != NULL);
     
    if(client->server.connection_state != TCP_Client_Connection_State_Connected){
        return TCP_Client_Result_Disconnected;
    }

    int totalBytesRead = 0;
    uint8_t temp_buf[2048];
    bool connection_closed = false;

    Socket_Result result = socket_read(&client->server.socket, temp_buf, sizeof(temp_buf), &totalBytesRead);
        
    if(result == Socket_Result_Nothing_Read){
        if(client->server.incoming_buffer_bytes == 0){

            sleep_ms(50);
            return TCP_Client_Result_Nothing_Read_Yet;
        }
    }

    if(result == socket_result_connection_closed){
        client->server.connection_state = TCP_Client_Connection_State_Disconnected;
        connection_closed = true;
        if(client->on_disconnect_callback){
            client->on_disconnect_callback(client);
        }
        if(totalBytesRead == 0 && client->server.incoming_buffer_bytes == 0){
            return TCP_Client_Result_Disconnected;
        }
    } 

    if(result == socket_result_error){
        if(client->on_error_callback){
            client->on_error_callback(client, TCP_Client_Result_Error_Socket_Read);
        }        
        return TCP_Client_Result_Error_Socket_Read;
    }

    if(totalBytesRead > 0){
        if((uint32_t)client->server.incoming_buffer_bytes + (uint32_t)totalBytesRead > sizeof(client->server.incoming_buffer)){
            if(client->on_error_callback){
                client->on_error_callback(client, TCP_Client_Result_Error_BufferOverFlow);
            }
            return TCP_Client_Result_Error_BufferOverFlow;
        }
        
        memcpy(client->server.incoming_buffer + client->server.incoming_buffer_bytes, temp_buf, (size_t)totalBytesRead);
        client->server.incoming_buffer_bytes += (uint32_t)totalBytesRead;

    }

    bool process_done = false;

    while(client->server.incoming_buffer_bytes > 0){
        uint8_t *buffer = client->server.incoming_buffer;
        uint32_t buffer_length = client->server.incoming_buffer_bytes;

        int headers_end_index = find_headers_end(buffer, buffer_length);
        if(headers_end_index < 0){
            break;
        }

        bool is_response = (buffer_length >= 5 && memcmp(buffer, "HTTP/", 5) == 0);

        int content_length = parse_content_length(buffer, (uint32_t)headers_end_index);
        uint32_t total_message_length = (uint32_t)headers_end_index;
        if(content_length > 0){
            uint32_t needed = (uint32_t)headers_end_index + (uint32_t)content_length;
            if(buffer_length < needed){
                break;
            }
            total_message_length = needed;
        } 

        process_done = true;

        if(is_response){
            if(client->on_received_callback){
                client->on_received_callback(client, buffer, total_message_length);
            }
        } else {
            char *temp = (char*)malloc(total_message_length + 1);
            if(!temp){
                if(client->on_error_callback){
                    client->on_error_callback(client, TCP_Client_Result_Error_Reading);
                }
                return TCP_Client_Result_Error_Reading;
            }
            memcpy(temp, buffer, total_message_length);
            temp[total_message_length] = '\0';

            Http_Request *request = Http_Parser_Parse(temp);
            free(temp);

            if(request){
                if(client->on_full_request_callback){
                    client->on_full_request_callback(client, request);
                }
                Http_Parser_Cleanup(request);
                free(request);
            } else {
                if(client->on_received_callback){
                    client->on_received_callback(client, buffer, total_message_length);
                }
            }           
        }

        uint32_t remaining = buffer_length - total_message_length;
        if(remaining > 0){
            memmove(client->server.incoming_buffer, client->server.incoming_buffer + total_message_length, remaining);
        }
        client->server.incoming_buffer_bytes = remaining;

    }

    if(connection_closed && client->server.incoming_buffer_bytes == 0){
        return TCP_Client_Result_Disconnected;
    }
        
    
    return process_done ? TCP_Client_Result_OK : TCP_Client_Result_Nothing_Read_Yet;
    
}


TCP_Client_Result tcp_client_send_queued(TCP_Client *client){

    assert(client != NULL);
    
    if(client->server.connection_state != TCP_Client_Connection_State_Connected){
        return TCP_Client_Result_Disconnecting;
    }

    if(client->server.outgoing_buffer_bytes == 0){
        client->working = false;
        return TCP_Client_Result_OK;
    }

    if(client->working){
        return TCP_Client_Result_Nothing_Sent_Yet;
    }

    client->working = true;

    uint32_t sent = 0;

    Socket_Result write_result = socket_write(&client->server.socket, client->server.outgoing_buffer, client->server.outgoing_buffer_bytes, &sent);

    if(write_result != Socket_Result_OK){
        client->working = false;
        if(write_result == socket_result_connection_closed){
            client->server.connection_state = TCP_Client_Connection_State_Disconnected;
            if(client->on_disconnect_callback){
                client->on_disconnect_callback(client);
            }
            return TCP_Client_Result_Disconnected;
        }

        if(write_result == Socket_Result_Nothing_Written_Yet){
            return TCP_Client_Result_Nothing_Sent_Yet;
        }
        return TCP_Client_Result_Error_Socket_Write;
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

    if(client->server.outgoing_buffer_bytes == 0){
        client->working = false;
    }

    return TCP_Client_Result_OK;
}


TCP_Client_Result tcp_client_send(TCP_Client *client, const uint8_t *buffer, uint32_t size){

    assert(client != NULL);
    assert(buffer != NULL);

    if(client->server.connection_state != TCP_Client_Connection_State_Connecting && client->server.connection_state != TCP_Client_Connection_State_Connected){
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
    client->server.close_requested = true;

    return TCP_Client_Result_OK;    
}

