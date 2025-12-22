#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdbool.h>
#include <unistd.h>
#include "../shared/tcp_shared.h"
#include "../../weather/weather.h"
#include "../../http/parser.h"
#include "../../utils/sleep.h"

#ifndef TCP_CLIENT_RECEIVE_BUFFER_SIZE
    #define TCP_CLIENT_RECEIVE_BUFFER_SIZE 2048 * 4
#endif

#ifndef TCP_CLIENT_OUTGOING_BUFFER_SIZE
    #define TCP_CLIENT_OUTGOING_BUFFER_SIZE 2048
#endif

typedef struct TCP_Client TCP_Client;
typedef struct TCP_Client_Server TCP_Client_Server;

typedef enum {
    
    TCP_Client_Connection_State_Disconnected,
    TCP_Client_Connection_State_Connecting,
    TCP_Client_Connection_State_Connected,
    TCP_Client_Connection_State_Disconnecting,

} TCP_Client_Connection_State;

typedef enum {
    
    TCP_Client_Result_OK,
    TCP_Client_Result_Already_Connected,
    TCP_Client_Result_Already_Initiated,
    TCP_Client_Result_Already_Working,
    TCP_Client_Result_Disconnecting,
    TCP_Client_Result_Disconnected,
    TCP_Client_Result_Nothing_Read_Yet,
    TCP_Client_Result_Nothing_Sent_Yet,

    TCP_Client_Result_Error_Connection_Failure,
    TCP_Client_Result_Error_Creating_Socket,
    TCP_Client_Result_Error_Socket_Read,
    TCP_Client_Result_Error_Reading,
    TCP_Client_Result_Error_Socket_Write,
    TCP_Client_Result_Error_Sending_Queued,
    TCP_Client_Result_Error_Fcntl,
    TCP_Client_Result_Error_Invalid_Address,
    TCP_Client_Result_Error_Trying_To_Work_Again,
    TCP_Client_Result_Error_BufferOverFlow,
    
    TCP_Client_Result_Not_Enough_Space,

    /* Maybe clean this up a bit, or separate them into two different enums, one for errors and one for other? (PL) */

} TCP_Client_Result;

/* ======================== CALLBACKS ======================== */
typedef void(*TCP_Client_Callback_On_Connect)(TCP_Client *client);
typedef void(*TCP_Client_Callback_On_Disconnect)(TCP_Client *client);
typedef void(*TCP_Client_Callback_On_Received_Bytes_From_Server)(TCP_Client *client, const uint8_t *buffer, const uint32_t buffer_size);
typedef void(*TCP_Client_Callback_On_Full_Request)(TCP_Client *client, Http_Request *request);
typedef void(*TCP_Client_Callback_On_Error)(TCP_Client *client, TCP_Client_Result error);

struct TCP_Client_Server {

    Socket socket;

    TCP_Client_Connection_State connection_state;
    
    uint8_t incoming_buffer[TCP_CLIENT_RECEIVE_BUFFER_SIZE];
    uint8_t outgoing_buffer[TCP_CLIENT_OUTGOING_BUFFER_SIZE];
    uint32_t incoming_buffer_bytes;
    uint32_t outgoing_buffer_bytes;

    bool close_requested;

};

struct TCP_Client {    
       
    bool working;    

    TCP_Client_Server server;

    void* context;
    
    TCP_Client_Callback_On_Received_Bytes_From_Server on_received_callback;
    TCP_Client_Callback_On_Full_Request on_full_request_callback;
    TCP_Client_Callback_On_Connect on_connect_callback;
    TCP_Client_Callback_On_Disconnect on_disconnect_callback;
    TCP_Client_Callback_On_Error on_error_callback;
};

TCP_Client_Result tcp_client_init(TCP_Client *client, void* _Context, TCP_Client_Callback_On_Received_Bytes_From_Server on_received, TCP_Client_Callback_On_Full_Request on_full_request, TCP_Client_Callback_On_Connect on_connect, TCP_Client_Callback_On_Disconnect on_disconnect, TCP_Client_Callback_On_Error on_error);

TCP_Client_Result tcp_client_connect(TCP_Client *client, const char *ip, int port);

TCP_Client_Result tcp_client_work(TCP_Client *client);

TCP_Client_Result tcp_client_send(TCP_Client *client, const uint8_t *buffer, uint32_t size);

TCP_Client_Result tcp_client_disconnect(TCP_Client *client);


#endif