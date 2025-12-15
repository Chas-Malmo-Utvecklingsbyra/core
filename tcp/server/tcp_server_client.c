#include "tcp_server_client.h"
#include "../../utils/clock_monotonic.h"

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

bool tcp_server_client_should_close(TCP_Server_Client* client)
{
    if (client->close_connection == true)
    {
        socket_close(&client->socket);
        return true;
    }
    return false;
}