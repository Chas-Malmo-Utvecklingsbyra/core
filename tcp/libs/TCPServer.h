#ifndef __TCPServer_h_
#define __TCPServer_h_

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "TCPClient.h"

#define MAX_CLIENTS 10

typedef struct
{
    int listen_fd;
    TCPClient clients[MAX_CLIENTS];

} TCPServer;

int TCPServer_Initiate(TCPServer* s, const char* port);

int TCPServer_Accept(TCPServer* s);

void TCPServer_Work(TCPServer* s);

void TCPServer_Dispose(TCPServer* s);

int TCPServer_Nonblocking(int fd);

#endif /* __TCPServer_h_ */