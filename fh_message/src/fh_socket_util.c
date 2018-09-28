/**
 * fh_socket_util.c
 *
 * Provides utility functions for socket management.
 */
#ifndef DISABLE_SOCKETS

#include <arpa/inet.h>
#include <sys/socket.h>

#include "fh_classes.h"

// forwards
void _fh_socket_util_lookup_local(int sock_fd, struct sockaddr_in *sock_address);
void _fh_socket_util_lookup_remote(int sock_fd, struct sockaddr_in *sock_address);

// open a tcpip client connection
// Returns the socket file descriptor on success, -1 on failure.
int
fh_socket_util_connect_tcpip(char *server_ip, int port)
{

    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket\n");
        return -1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_port = htons(port);

    int status = connect(socket_desc, (struct sockaddr *)&server, sizeof(server));
    if (status < 0) {
        printf("Could not connect to server at %s:%d\n", server_ip, port);
        return -1;
    }

    return socket_desc;
}

// open a tcpip server socket on the local port
int
fh_socket_util_open_server_socket(int port)
{
    int socket_desc;
    struct sockaddr_in server;

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // don't fail for socket in TIME_WAIT
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server))) {
        printf("Could not open socket on port %d\n", port);
        return -1;
    }

    listen(socket_desc, 3);

    return socket_desc;
}

// wait for a connection on a server socket
int
fh_socket_util_wait_for_connect(int server_sock_fd)
{
    struct sockaddr_in client;

    int c = sizeof(struct sockaddr_in);
    int conn = accept(server_sock_fd, (struct sockaddr *)&client, (socklen_t *)&c);

    // char *client_ip = inet_ntoa(client.sin_addr);
    // int client_port = ntohs(client.sin_port);

    return conn;
}

// extract the local ip address from a socket descriptor
char *
fh_socket_util_extract_local_ip(int sock_fd, char *buffer)
{
    struct sockaddr_in sock_address;
    _fh_socket_util_lookup_local(sock_fd, &sock_address);

    // todo convert to ipv6 formulation
    // inet_ntop(AF_INET, sock_address.sin_addr)

    char *ip = inet_ntoa(sock_address.sin_addr);
    strcpy(buffer, ip);
    return buffer;
}

// extract the remote ip address from a socket descriptor
char *
fh_socket_util_extract_remote_ip(int sock_fd, char *buffer)
{
    struct sockaddr_in sock_address;
    _fh_socket_util_lookup_remote(sock_fd, &sock_address);

    // todo convert to ipv6 formulation
    // inet_ntop(AF_INET, sock_address.sin_addr)

    char *ip = inet_ntoa(sock_address.sin_addr);
    strcpy(buffer, ip);
    return buffer;
}

// extract the local port number from a server socket descriptor
uint16_t
fh_socket_util_extract_local_port(int sock_fd)
{
    struct sockaddr_in sock_address;
    _fh_socket_util_lookup_local(sock_fd, &sock_address);
    return ntohs(sock_address.sin_port);
}

// extract the remote port number from a socket descriptor
uint16_t
fh_socket_util_extract_remote_port(int sock_fd)
{
    struct sockaddr_in sock_address;
    _fh_socket_util_lookup_remote(sock_fd, &sock_address);
    return ntohs(sock_address.sin_port);
}

// look up the socket details from a socket descriptor
void
_fh_socket_util_lookup_local(int sock_fd, struct sockaddr_in *sock_address)
{
    int c = sizeof(sock_address);
    getsockname(sock_fd, (struct sockaddr *)sock_address, (socklen_t *)&c);
}

// look up the socket details from a socket descriptor
void
_fh_socket_util_lookup_remote(int sock_fd, struct sockaddr_in *sock_address)
{
    int c = sizeof(sock_address);
    getpeername(sock_fd, (struct sockaddr *)sock_address, (socklen_t *)&c);
}
#else
void nop(void);
#endif
