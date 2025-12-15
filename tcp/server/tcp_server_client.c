#include "tcp_server_client.h"
#include "../../utils/clock_monotonic.h"

#include <stdio.h>
#include <assert.h>

void tcp_server_client_init(TCP_Server_Client* client)
{
    client->in_use = false;
}

void tcp_server_client_dispose(TCP_Server_Client* client)
{
	close(client->socket.file_descriptor);
}

bool tcp_server_client_should_timeout(TCP_Server_Client* client)
{
    if (client->timestamp + 15000 < SystemMonotonicMS())
    {
        client->close_connection = true;
        return true;
    }
    return false;
}

bool tcp_server_client_get_accepted(TCP_Server_Client* client, int cfd)
{
    if (client->in_use == false)
    {
        /* client->unique_id = i; */
        client->in_use = true;
        client->socket.file_descriptor = cfd;
        client->timestamp = SystemMonotonicMS();
        return true;
    }

    return false;
}

bool tcp_server_client_send(TCP_Server_Client* client)
{
    if (client->outgoing_buffer_amount_of_bytes == 0)
    {
        return false;
    }

    uint32_t totalBytesSent = 0;

    Socket_Result write_result = socket_write(&client->socket, client->outgoing_buffer, client->outgoing_buffer_amount_of_bytes, &totalBytesSent);

    if (write_result == socket_result_connection_closed)
    {
        printf("%s, Socket closed FD: %i\n", __FILE__, client->socket.file_descriptor);
        return false;
    }

    assert(totalBytesSent <= client->outgoing_buffer_amount_of_bytes);

    // printf("total bytes send: %u\n", totalBytesSent);
    /* TODO: 1 2 3 4 5 6 7 8 */
    uint32_t y;
    for (y = 0; y < client->outgoing_buffer_amount_of_bytes - totalBytesSent; y++)
    {
        client->outgoing_buffer[y] = client->outgoing_buffer[totalBytesSent + y];
    }
    client->outgoing_buffer[totalBytesSent + 1] = '\0';

    client->outgoing_buffer_amount_of_bytes -= totalBytesSent;

    //printf("Buffer amount of bytes: %u\n", client->outgoing_buffer_amount_of_bytes);
        /* server->on_received_bytes_from_client(server, client, &client->receive_buffer[0], totalBytesRead); */

    return true;
}

bool tcp_server_client_should_close(TCP_Server_Client* client)
{
    if (client->close_connection == true)
    {
        socket_close(&client->socket);
        return true;
    }
    return false;
}