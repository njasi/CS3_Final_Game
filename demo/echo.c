#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "network_util.h"

int conn;

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4 || (strcmp(argv[1], "server") != 0 && strcmp(argv[1], "client") != 0)) {
        fprintf(stderr, "Usage.  There are two modes: server and client.\n");
        fprintf(stderr, "Server Mode:\n");
        fprintf(stderr, "  %s server <server port>\n", argv[0]);
        fprintf(stderr, "Client Mode:\n");
        fprintf(stderr, "  %s client <server ip> <server port>\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "server") == 0) {
        printf("Waiting for a client to connect...\n");
        conn = nu_wait_client(atoi(argv[2]));
        printf("Client connected!\n");
    }
    else {
        conn = nu_connect_server(argv[2], atoi(argv[3]));
        printf("Connected to server!\n");
    }

    if (conn < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }

    printf("Type something and it will be transferred to the other party!\nType 'quit' to end the connection.\n");

    while (1) {
        char *remote = nu_try_read_str(conn);
        if (remote != NULL) {
            if (strcmp(remote, "quit") == 0) {
                free(remote);
                nu_close_connection(conn);
                exit(0);
            }
            else {
                printf("%s\n", remote);
            }
            free(remote);
        }
        char *local = nu_try_read_str(STDIN_FILENO);
        if (local != NULL) {
            nu_send_str(conn, local);
            if (strcmp(local, "quit") == 0) {
                free(local);
                nu_close_connection(conn);
                exit(0);
            }
            free(local);
        }
    }

    return 0;
}
