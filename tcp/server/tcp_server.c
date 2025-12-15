#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "tcp_server.h"

#include <assert.h>
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
#include <stdlib.h>
#include "../../utils/min.h" /* TODO: Fix path in makefile */
#include "../../utils/clock_monotonic.h"

/* private functions */
TCP_Server_Result tcp_server_accept(TCP_Server *server);
TCP_Server_Result tcp_server_read(TCP_Server *server);
TCP_Server_Result tcp_server_send(TCP_Server *server);
TCP_Server_Result tcp_server_close_connection(TCP_Server *server);
TCP_Server_Result tcp_server_timeout_checker(TCP_Server *server);


/*  initiate server 
	provide port in range of 0 to 65535 */
TCP_Server_Result tcp_server_init(TCP_Server *server, TCP_Server_Callback_On_Recieved_Bytes_From_Client on_received_bytes_from_client) {
	
	/* moved from tcp_server_start and stored in TCP_Server struct */
	struct addrinfo* hints = malloc(sizeof(struct addrinfo));
	if (hints == NULL)
	{
		return TCP_Server_Result_Error;
	}
	memset(hints, 0, sizeof(struct addrinfo));

	hints->ai_family = AF_INET;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_flags = AI_PASSIVE;

	server->hints = hints; /* store in server struct */

	server->on_received_bytes_from_client = on_received_bytes_from_client;

	/* moved from _start*/
	int i;
	for (i = 0; i < TCP_MAX_CLIENTS_PER_SERVER; i++)
		tcp_server_client_init(&server->clients[i]);
	
	/* reset tracking counters */
	server->client_count = 0;
	server->next_unique_id = 0;

	return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_start(TCP_Server *server, uint16_t port){
	/* TODO: SS - Move some of the things from here to tcp_server_init(..). */
	server->port = port;

    struct addrinfo *res = NULL;

	char port_buf[6];
	memset(port_buf, 0, sizeof(port_buf));
	snprintf(port_buf, sizeof(port_buf), "%d", server->port);

	if (getaddrinfo(NULL, port_buf, server->hints, &res) != 0)
		return TCP_Server_Result_Error;

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
		return TCP_Server_Result_Bind_Failure;


	if (listen(fd, TCP_MAX_CLIENTS_PER_SERVER) < 0)
	{
		close(fd);
		return TCP_Server_Result_Listen_Failure;
	}

    int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return TCP_Server_Result_Error;

	/* return fcntl(fd, F_SETFL, flags | O_NONBLOCK); */
	int fcntl_result = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if (fcntl_result == -1)
	{
		printf("errno: %d\n", errno);
		return TCP_Server_Result_Error;
	}

	server->socket.file_descriptor = (uint32_t)fd;

	printf("Server listening to %d\n", port);

    return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_work(TCP_Server *server){

	TCP_Server_Result server_accept_result = tcp_server_accept(server);
	if (server_accept_result != TCP_Server_Result_OK && server_accept_result != TCP_Server_Result_Server_Full)
	{
		return server_accept_result;
	}

	TCP_Server_Result server_read_result = tcp_server_read(server);
	if (server_read_result != TCP_Server_Result_OK)
	{
		/* printf("Failed to read TCP server. Result: %i.\n", server_read_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result) */
		return server_read_result;
	}
	TCP_Server_Result server_send_result = tcp_server_send(server);
	if (server_send_result != TCP_Server_Result_OK)
	{
		/* printf("Failed to read TCP server. Result: %i.\n", server_read_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result) */
		return server_send_result;
	}

	TCP_Server_Result server_timeout_result = tcp_server_timeout_checker(server);
	if (server_timeout_result != TCP_Server_Result_OK)
	{
		/* printf("Failed to read TCP server. Result: %i.\n", server_read_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result) */
		return server_timeout_result;
	}

	TCP_Server_Result server_clean_result = tcp_server_close_connection(server);
	if (server_clean_result != TCP_Server_Result_OK)
	{
		/* printf("Failed to read TCP server. Result: %i.\n", server_read_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result) */
		return server_clean_result;
	}

	return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_send(TCP_Server *server){
	size_t i;
	for (i = 0; i < server->client_count; i++)
	{
		tcp_server_client_send(&server->clients[i]);
	}

	return TCP_Server_Result_OK;

}

TCP_Server_Result tcp_server_accept(TCP_Server *server){

	/* Make into function - try_accept_new_socket */
    int cfd = accept(server->socket.file_descriptor, NULL, NULL);
	if (cfd < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0; /* No new client */

		perror("accept");
		return -1;
	}
	/* end */

	/* set potential socket */

	/* set_socket_blocking */
	int flags = fcntl(cfd, F_GETFL, 0);
	if (flags < 0)
		return -1;
	fcntl(cfd, F_SETFL, flags | O_NONBLOCK);
	/* End */

	/* Find free socket */
	uint32_t i;
	for (i = 0; i < TCP_MAX_CLIENTS_PER_SERVER; i++) {
		
		TCP_Server_Client *client = &server->clients[i];

		if (tcp_server_client_get_accepted(client, cfd))
		{
			server->client_count++;
			printf("Ny klient accepterad (FD: %d, index: %i/%i)\n", client->socket.file_descriptor, i+1, TCP_MAX_CLIENTS_PER_SERVER);
			return TCP_Server_Result_OK;
		}
	}

	/* FULL */
	/* socket_close */
	close(cfd);
	printf("Max klienter, anslutning avvisad\n");

    return TCP_Server_Result_Server_Full;
}

TCP_Server_Result tcp_server_read(TCP_Server *server)
{
	size_t i;
	for (i = 0; i < server->client_count; i++)
	{
		TCP_Server_Client *client = &server->clients[i];
		
		int totalBytesRead = 0;
		/* TODO: Fix buffer read from position of already existing read data */
		Socket_Result read_result = socket_read(&client->socket, &client->receive_buffer[0], TCP_MAX_CLIENT_BUFFER_SIZE, &totalBytesRead);

		if (read_result != Socket_Result_OK)
		{
			/* printf("%s, read_result != ok\n", __FILE__); */
			continue;
		}
		if(totalBytesRead > 0)
		{
			server->on_received_bytes_from_client(server, client, &client->receive_buffer[0], totalBytesRead);
		}
	}

	return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_send_to_client(TCP_Server *server, TCP_Server_Client *client, const uint8_t *buffer, const uint32_t buffer_size){
	
	(void)server;

	uint32_t outgoing_capacity_remaining = sizeof(client->outgoing_buffer) - client->outgoing_buffer_amount_of_bytes; 


	if (outgoing_capacity_remaining < buffer_size){
		return TCP_Server_Result_Not_Enough_Space;
	}

	uint32_t amount_of_bytes_to_send = min_uint32(buffer_size, outgoing_capacity_remaining);

	memcpy(&client->outgoing_buffer[client->outgoing_buffer_amount_of_bytes], buffer, amount_of_bytes_to_send);
	
	client->outgoing_buffer_amount_of_bytes += amount_of_bytes_to_send;

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
		tcp_server_client_dispose(&server->clients[i]);
	}

	close(server->socket.file_descriptor);
	free(server->hints);

    return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_timeout_checker(TCP_Server *server){
	
	uint32_t i;
	for(i = 0; i < server->client_count; i++){
		if (tcp_server_client_should_timeout(&server->clients[i]))
		{
			printf("updated CLOSE status on client: %i\n", i);
		}
	}
	return TCP_Server_Result_OK;
}


TCP_Server_Result tcp_server_close_connection(TCP_Server *server){
	
	uint32_t start_count = server->client_count;
	uint32_t i;
	for(i = 0; i < start_count; i++){

		if (tcp_server_client_should_close(&server->clients[i]))
		{
			server->clients[i] = server->clients[server->client_count-1];
			memset(&server->clients[server->client_count-1], 0, sizeof(TCP_Server_Client));
			server->client_count--;
			printf("closed client connection %i\n", i);
		}
	}
	return TCP_Server_Result_OK;
}