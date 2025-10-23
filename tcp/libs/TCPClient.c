

#include "TCPClient.h"

int TCPClient_Initiate(TCPClient* c)
{
	c->fd = -1;
	return 0;
}

int TCPClient_Connect(TCPClient* c, const char *host, const char *port)
{
    struct addrinfo hints = {0};
	struct addrinfo* res = NULL;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, port, &hints, &res) != 0)
        return -1;

	/*
    Prof. Andersson says:
    The getaddrinfo() function can provide a linked list of address suggestions for the same host and port.
    For example, a server may have both IPv4 and IPv6 addresses, or multiple network interfaces.
    
    Each node in the list (struct addrinfo) contains a possible address to try.
    If the first address doesn't work (e.g., connect() fails), the next one is attempted.
	*/
	
    int fd = -1;
    struct addrinfo *rp;
    for (rp = res; rp; rp = rp->ai_next)
	{
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (fd < 0)
			continue;

        if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;

        close(fd);
        fd = -1;
    }

    freeaddrinfo(res);
    if (fd < 0)
		return -1;

    c->fd = fd;
    return 0;
}

int TCPClient_Write(TCPClient* c, const uint8_t* buf, int len)
{
    return send(c->fd, buf, len, MSG_NOSIGNAL);
}

int TCPClient_Read(TCPClient* c, uint8_t* buf, int len)
{
    return recv(c->fd, buf, len, MSG_DONTWAIT); /* non blocking read */
}

void TCPClient_Disconnect(TCPClient* c)
{
    if (c->fd >= 0)
		close(c->fd);

    c->fd = -1;
}

void TCPClient_Dispose(TCPClient* c)
{
	TCPClient_Disconnect(c);
}