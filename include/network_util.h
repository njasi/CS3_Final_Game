#ifndef __NETWORK_UTIL_H
#define __NETWORK_UTIL_H

#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>


/**
 * Specifies the type of callback to be used in nu_multiplex.
 **/
typedef void nu_callback(int fd, char *str);

/**
 * Listens on localhost:port for a single client connection.
 * This function is blocking (i.e., it waits for a client to
 * connect before returning).
 **/
int nu_wait_client(int port);

/**
 * Opens a client connection to the server pointed to by ip:port.
 * This function assumes that the server has already 
 * called nu_wait_client.
 **/
int nu_connect_server(char *ip, int port);

/** 
 * Sends a string to the open connection represented by fd.
 **/
int nu_send_str(int fd, char *str);

/**
 * Attempts to read a string sent by nu_send_str from the open
 * connection represented by fd.
 * This function is non-blocking.
 **/
char *nu_try_read_str(int fd);

/**
 * Reads a string sent by nu_send_str from the open 
 * connection represented by fd.
 * This function is blocking.
 **/
char *nu_read_str(int fd);

/**
 * This function allows a client to simultaneously multiplex over two file descriptors
 * (which could be sockets or files).
 * This function is blocking.
 **/
int nu_multiplex(int localfd, int remotefd, nu_callback on_local_write, nu_callback on_remote_write);

/**
 * Closes a connection opened by nu_wait_client or nu_connect_server.
 **/
void nu_close_connection(int fd);

#endif /* __NETWORK_UTIL_H */
