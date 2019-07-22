#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "network_util.h"

#define INITIAL_BUFFER_SIZE 2
#define RESIZE_MULTIPLIER 2
#define LISTENQ 50

#define max(a,b) (a > b ? a : b)

int nu_server_listen(int port) {
    int myfd = 0;
    if ((myfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(myfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "bind to %s:%d failed (%s)\n", "localhost", port, strerror(errno));
        close(myfd); 
        return -1;
    }
    if (listen(myfd, LISTENQ) < 0) {
        fprintf(stderr, "listen on %s:%d failed\n", "localhost", port);
        close(myfd);
        return -1;
    }
    return myfd;
}

int nu_wait_client(int port) {
    int myfd = nu_server_listen(port);
    if (myfd < 0) {
        return -1;
    }

    struct sockaddr_in client_addr; 
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    int clientfd = accept(myfd, (struct sockaddr *) &client_addr, &client_addr_size); 
    if (clientfd < 0) {
        fprintf(stderr, "accept failed\n");
        close(myfd);
        return -1;
    }

    close(myfd);
    return clientfd;
}

int nu_connect_server(char *ip, int port) {
    int fd = 0;
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "pton %s:%d failed (%s)\n", ip, port, strerror(errno));
        close(fd); 
        return -1;
    }

    if (connect(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "connect to %s:%d failed (%s)\n", ip, port, strerror(errno));
        close(fd); 
        return -1;
    }
    return fd;
}

int nu_send_str(int fd, char *str) {
    int len = strlen(str) + 1;
    if (send(fd, str, len, 0) < 0) {
        return -1;
    }
    return len;
}

static int num_extra_chars;
static char extra_chars[INITIAL_BUFFER_SIZE];

int nu_check_for_terminator(char *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (buf[i] == '\n') {
            buf[i] = '\0';
        }
        if (buf[i] == '\0') {
            return i;
        }
    }
    return -1;
}

char *nu_read_str(int fd) {
    size_t current_buffer_size = INITIAL_BUFFER_SIZE;
    char *buf = malloc(current_buffer_size);

    memcpy(buf, extra_chars, num_extra_chars);
    char *curr = buf + num_extra_chars;

    while (1) {
        size_t to_read = INITIAL_BUFFER_SIZE - num_extra_chars;
        num_extra_chars = 0;
        ssize_t actually_read = read(fd, extra_chars, to_read);

        /* If there was an error, destroy the buffer and indicate 
         * an error has occurred. */
        if (actually_read < 0) {
            perror("recv");
            free(buf);
            return NULL;
        }

        /* If we're out of space, realloc the buffer to include 
         * more space. */
        if ((curr - buf) + actually_read > current_buffer_size) {
            buf = realloc(buf, current_buffer_size * RESIZE_MULTIPLIER); 
            curr = buf + current_buffer_size; 
            current_buffer_size *= RESIZE_MULTIPLIER;
        }

        memcpy(curr, extra_chars, actually_read);

        /* If the block of bytes has a null terminator in it, then we need
         * to finish the string and save the rest for the next call to
         * this function. */
        int terminator = nu_check_for_terminator(extra_chars, actually_read);
        if (terminator != -1) {
            num_extra_chars = actually_read - terminator - 1;
            memcpy(extra_chars, curr + terminator + 1, num_extra_chars);
            curr += terminator;
            *curr = '\0';
            return buf;
        }
        curr += actually_read;
    }

    // Execution should never reach here...
    assert(false);
}

char *nu_try_read_str(int fd) {
    fd_set rd;
    FD_ZERO(&rd);
    FD_SET(fd, &rd);

    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 100;

    int result = -1;
    if ((result = select(fd + 1, &rd, NULL, NULL, &tv)) < 0) {
        return NULL;
    }
    if (FD_ISSET(fd, &rd)) {
        return nu_read_str(fd);
    }
    return NULL;
}

void nu_close_connection(int fd) {
    close(fd);
}

int nu_multiplex(int localfd, int remotefd, nu_callback on_local_write, nu_callback on_remote_write) {
    fd_set rd;

    while (1) {
        FD_ZERO(&rd);
        FD_SET(localfd, &rd);
        FD_SET(remotefd, &rd);

        int result = -1;
        if ((result = select(max(localfd, remotefd) + 1, &rd, NULL, NULL, NULL)) < 0) {
            perror("select");
            return -1;
        }
        if (FD_ISSET(remotefd, &rd)) {
            on_remote_write(remotefd, nu_read_str(remotefd));
        }
        if (FD_ISSET(localfd, &rd)) {
            on_local_write(remotefd, nu_read_str(localfd));
        }
    }
}
