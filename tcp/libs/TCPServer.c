#include "TCPServer.h"
#include "TCPClient.h"

int TCPServer_Initiate(TCPServer* s, const char *port)
{
	struct addrinfo hints = {0}, *res = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &res) != 0)
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

	if (listen(fd, MAX_CLIENTS) < 0)
	{
		close(fd);
		return -1;
	}

	TCPServer_Nonblocking(fd);

	s->listen_fd = fd;
	int i;
	for (i = 0; i < MAX_CLIENTS; i++)
		s->clients[i].fd = -1;

	printf("Server lyssnar på port %s\n", port);

	return 0;
}

int TCPServer_Accept(TCPServer* s)
{
	int cfd = accept(s->listen_fd, NULL, NULL);
	if (cfd < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0; /* No new client */

		perror("accept");
		return -1;
	}

	TCPServer_Nonblocking(cfd);

	/* Find free socket */
	int i;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (s->clients[i].fd < 0)
		{
			s->clients[i].fd = cfd;
			printf("Ny klient accepterad (index %d)\n", i);
			return 1;
		}
	}

	/* FULL */
	close(cfd);
	printf("Max klienter, anslutning avvisad\n");
	return 0;
}

void TCPServer_Work(TCPServer* s)
{
	TCPServer_Accept(s);

	
	char buf[512];
	int i;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		int fd = s->clients[i].fd;
		if (fd < 0)
			continue;

		ssize_t n = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
		if (n > 0)
		{
			send(fd, buf, (size_t)n, MSG_NOSIGNAL); /* echo client */
		}
		else if (n == 0)
		{
			printf("Klient %d kopplade ner\n", i);
			close(fd);
			s->clients[i].fd = -1;
		}
	}
}

int TCPServer_Nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return -1;

	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void TCPServer_Dispose(TCPServer* s)
{
	close(s->listen_fd);
	int i;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (s->clients[i].fd >= 0)
			close(s->clients[i].fd);

	}
}
