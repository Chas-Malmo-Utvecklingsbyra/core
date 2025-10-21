#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>  /* make accept not block */

#include "tcp.h"
#include "smw.h"

struct TcpServer {
    int server_fd;
    int port;
    ClientHandler handler;
};
/* START OF NEW STUFF USING STATEMACHINEWORKER START OF */

/* struct to hold socket and temporary buffer for each client 
updated to receive ClientHander handler from main.c (i think)*/
typedef struct {
    int fd;
    char buffer[1024];
    ClientHandler handler;
    smw_task* task;
}ClientSession;

/* client session call back task !?!? */
void client_task(void* _Context, uint64_t _MonTime) {
    ClientSession* session = (ClientSession*)_Context;
    ssize_t bytes = recv(session->fd, session->buffer, sizeof(session->buffer) - 1, MSG_DONTWAIT);

    if (bytes > 0) {
        session->buffer[bytes] = '\0';
        session->handler(session->fd, session->buffer);
    } else if (bytes == 0 || (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
        close(session->fd);
        if (session->task) {
            smw_destroyTask(session->task);
        }
        free(session);
    }
    (void)_MonTime; /*suppress unused warning*/
}

/* make accept non blocking */
void accept_task(void* _Context, uint64_t _MonTime) {
    TcpServer* server = (TcpServer*)_Context;

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &addr_len);

    if (client_fd >= 0) {
        ClientSession* session = malloc(sizeof(ClientSession));
        session->fd = client_fd;
        session->handler = server->handler;

        smw_task* task = smw_createTask(session, client_task);
        
        if (task) {
            session->task = task;
        } else {
            close(client_fd);
            free(session);
        }
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("accept");
    }

    (void)_MonTime; /*suppress unused warning*/
}



/* END OF NEW STUFF USING STATEMACHINEWORKER END OF */

TcpServer* tcp_server_create(int port, ClientHandler handler) {
    TcpServer* server = malloc(sizeof(TcpServer));
    if (!server) return NULL;

    server->port = port;
    server->handler = handler;

    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd < 0) {
        perror("socket");
        free(server);
        return NULL;
    }

    int opt = 1;
    setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };

    if (bind(server->server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server->server_fd);
        free(server);
        return NULL;
    }

    if (listen(server->server_fd, 10) < 0) {
        perror("listen");
        close(server->server_fd);
        free(server);
        return NULL;
    }

    return server;
}
/*NEW*/
int tcp_server_run(TcpServer* server) {
    if (!server) return -1;

    smw_init();
    printf("Server listening on port %d...\n", server->port);

    bool keepServerOn = true;
    fcntl(server->server_fd, F_SETFL, O_NONBLOCK); /* make accept not block ? */

    /*Register accept task*/
    smw_init();
    smw_createTask(server, accept_task);

    printf("Server listening on port %d...\n", server->port);

    while (keepServerOn) {
        smw_work(0);
        usleep(10000);
    }


    smw_dispose();
    return 0;
}

/*END OF NEW*/
/* OLD tcp_server_run
int tcp_server_run(TcpServer* server) {
    if (!server) return -1;
    printf("Server listening on port %d...\n", server->port);
    bool keepServerOn = true;
    while (keepServerOn) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        char buffer[1024];
        ssize_t bytes;
        while ((bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0){
            if (bytes > 0) {
                buffer[bytes] = '\0';
                if (server->handler) {
                    server->handler(client_fd, buffer);
                }
            }
            if(strcmp(buffer, "terminate server\n") == 0){
                keepServerOn = false;
                break;
            }
        }
        close(client_fd);
    }
    return 0;
}
*/

void tcp_server_send(int clientfd, const char *message) {
    if (clientfd <= 0 || message == NULL) return;

    ssize_t sent = send(clientfd, message, strlen(message), 0);
    if (sent < 0) {
        perror("send failed");
    }
}

void tcp_server_destroy(TcpServer* server) {
    if (!server) return;
    close(server->server_fd);
    free(server);
}