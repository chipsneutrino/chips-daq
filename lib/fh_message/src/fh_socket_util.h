/**
 * fh_socket_util.h
 *
 * Provides utility functions for socket management.
 */

#ifndef FH_SOCKET_UTIL_INCLUDED
#define FH_SOCKET_UTIL_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// open a tcpip client connection
// Returns the socket file descriptor on success, -1 on failure.
int
fh_socket_util_connect_tcpip(char* server_ip, int port);

// open a tcpip server socket on the local port
int
fh_socket_util_open_server_socket(int port);

// wait for a connection on a server socket
int
fh_socket_util_wait_for_connect(int server_sock_fd);

// extract the local ip address from a socket descriptor
char*
fh_socket_util_extract_local_ip(int sock_fd, char* buffer);

// extract the remote ip address from a socket descriptor
char*
fh_socket_util_extract_remote_ip(int sock_fd, char* buffer);

// extract the local port number from a server socket descriptor
uint16_t
fh_socket_util_extract_local_port(int sock_fd);

// extract the remote port number from a socket descriptor
uint16_t
fh_socket_util_extract_remote_port(int sock_fd);


#ifdef __cplusplus
}
#endif

#endif
