#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "tcp_server.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdint.h>

TCP_Server_Result tcp_server_init(TCP_Server *server, TCP_Server_Callback_On_Recieved_Bytes_From_Client on_received_bytes_from_client) {
	(void)server;
	(void)on_received_bytes_from_client;

	server->port = 8080; // TODO: SS - Make this customizable.
	server->on_received_bytes_from_client = on_received_bytes_from_client;

    return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_start(TCP_Server *server){
	// TODO: SS - Move some of the things from here to tcp_server_init(..).

    struct addrinfo hints = {0};
    struct addrinfo *res = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	const char *port_as_string = "8080"; // TEMP: SS - This should not be hardcoded.
	if (getaddrinfo(NULL, port_as_string, &hints, &res) != 0)
		return -1;

	int fd = -1;
	struct addrinfo *rp;
	for(rp = res; rp; rp = rp->ai_next)
	{
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd < 0)
			continue;

		int yes = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;

		close(fd);
		fd = -1;
	}

	freeaddrinfo(res);
	if (fd < 0)
		return -1;

	if (listen(fd, TCP_MAX_CLIENTS_PER_SERVER) < 0)
	{
		close(fd);
		return -1;
	}

    int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return -1;

	/*return fcntl(fd, F_SETFL, flags | O_NONBLOCK);*/
	int fcntl_result = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if (fcntl_result == -1)
	{
		printf("errno: %d\n", errno);
		return -1;
	}

	server->socket.file_descriptor = (uint32_t)fd;

	int i;
	for (i = 0; i < TCP_MAX_CLIENTS_PER_SERVER; i++)
		server->clients[i].socket.file_descriptor = -1;

	printf("Server lyssnar på port %u\n", server->port);

    return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_accept(TCP_Server *server){
    
    int cfd = accept(server->socket.file_descriptor, NULL, NULL);
	if (cfd < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0; /* No new client */

		perror("accept");
		return -1;
	}

    
	int flags = fcntl(cfd, F_GETFL, 0);
	if (flags < 0)
		return -1;
	fcntl(cfd, F_SETFL, flags | O_NONBLOCK);
	server->client_count++;

	/* Find free socket */
	uint32_t i;
	for (i = 0; i < server->client_count; i++) {
		TCP_Server_Client *client = &server->clients[i];

		client->unique_id = i;
		client->socket.file_descriptor = cfd;
		printf("Ny klient accepterad (index %d)\n", client->unique_id);
		return TCP_Server_Result_OK;
	}

	/* FULL */
	close(cfd);
	printf("Max klienter, anslutning avvisad\n");

    return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_read(TCP_Server *server)
{
	size_t i;
	for (i = 0; i < server->client_count; i++)
	{
		TCP_Server_Client *client = &server->clients[i];

		Socket_Result read_result = socket_read(&client->socket, &client->receive_buffer[0], TCP_MAX_CLIENT_BUFFER_SIZE);

		if (read_result != Socket_Result_OK)
		{
			printf("%s, read_result != ok\n", __FILE__);
			continue;
		}

		server->on_received_bytes_from_client(server, client, &client->receive_buffer[0], TCP_MAX_CLIENT_BUFFER_SIZE);
	}

	return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_stop(TCP_Server *server) {
	(void)server;

	printf("TODO: SS - Add 'tcp_server_stop'.\n");
    return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_dispose(TCP_Server *server){
	uint32_t i;
	for (i = 0; i < server->client_count; i++)
	{
		TCP_Server_Client *client = &server->clients[i];
		close(client->socket.file_descriptor);
	}

	close(server->socket.file_descriptor);
	
    return TCP_Server_Result_OK;
}