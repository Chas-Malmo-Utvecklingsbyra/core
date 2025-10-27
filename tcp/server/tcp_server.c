#define _POSIX_C_SOURCE 200809L

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

#include "tcp_server.h"




TCP_Server_Result tcp_server_start(TCP_Server *server){

    struct addrinfo hints = {0};
    struct addrinfo *res = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, server->port, &hints, &res) != 0)
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

	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	server->socket.file_descriptor = (uint32_t*)fd;
	int i;
	for (i = 0; i < TCP_MAX_CLIENTS_PER_SERVER; i++)
		server->clients[i].socket.file_descriptor = -1;

	printf("Server lyssnar på port %s\n", server->port);

	return 0;




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

	/* Find free socket */
	int i;
	for (i = 0; i < TCP_MAX_CLIENTS_PER_SERVER; i++)
	{
		if (server->clients[i].socket.file_descriptor < 0)
		{   
            server->clients[i].unique_id = i;
			server->clients[i].socket.file_descriptor = cfd;
			printf("Ny klient accepterad (index %d)\n", server->clients[i].unique_id);
			return 1;
		}
	}

	/* FULL */
	close(cfd);
	printf("Max klienter, anslutning avvisad\n");


    return TCP_Server_Result_OK;
}

TCP_Server_Result tcp_server_stop(TCP_Server *server);

TCP_Server_Result tcp_server_dispose(TCP_Server *server){

    	close(server->socket.file_descriptor);
	int i;
	for (i = 0; i < TCP_MAX_CLIENTS_PER_SERVER; i++)
	{
		if (server->clients[i].socket.file_descriptor >= 0)
			close(server->clients[i].socket.file_descriptor);

	}
}


