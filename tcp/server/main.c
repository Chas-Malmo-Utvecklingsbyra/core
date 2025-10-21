#include "tcp.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef void (*CommandHandler)(int _ClientFD);

typedef struct {
    const char *command;
    CommandHandler handler;
} CommandEntry;

void handle_weather(int _ClientFD) {
    tcp_server_send(_ClientFD, "Please see the attached weather report\n");
}

void handle_config(int _ClientFD) {
    tcp_server_send(_ClientFD, "Sending a default client config\n");
}

void handle_question(int _ClientFD) {
    tcp_server_send(_ClientFD, "42\n");
}

void handle_kenobi(int _ClientFD) {
    tcp_server_send(_ClientFD, "Ah, general Kenobi!\n");
}

void handle_default(int _ClientFD) {
    tcp_server_send(_ClientFD, "default: Message received.\n");
}

CommandEntry command_table[] = {
    { "weather please\n", handle_weather },
    { "config please\n", handle_config },
    { "what is the meaning of life?\n", handle_question },
    { "hello there\n", handle_kenobi },
    { NULL, NULL } /* Sentinel */
};


void handle_client(int _ClientFD, const char* _Message) {

    printf("Received: %s\n", _Message);
    int i;
    for (i = 0; command_table[i].command != NULL; i++) {
        if (strcmp(_Message, command_table[i].command) == 0) {
            command_table[i].handler(_ClientFD);
            return;
        }
    }
    handle_default(_ClientFD);
}

int main() {
    TcpServer* server = tcp_server_create(8080, handle_client);
    if (!server) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    tcp_server_run(server);
    
    tcp_server_destroy(server);
    return 0;
}