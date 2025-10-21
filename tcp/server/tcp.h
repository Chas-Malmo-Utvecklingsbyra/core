#ifndef __TCP_H__
#define __TCP_H__

#include <netinet/in.h>

typedef struct TcpServer TcpServer;

/*Callback for handling client connections*/
typedef void (*ClientHandler)(int client_fd, const char* message);

/*Create and start a TCP server*/
TcpServer* tcp_server_create(int port, ClientHandler handler);

/*Run the server loop (blocking)*/
int tcp_server_run(TcpServer* server);

/* server send to client */
void tcp_server_send(int clientfd, const char *message);

/*Stop and clean up the server*/
void tcp_server_destroy(TcpServer* server);

#endif /*end of __TCP_H__*/