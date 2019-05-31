/**
 * fh_connector.c
 *
 * Encapsulates establishing a connection.
 *
 */

#include "fh_classes.h"

typedef fh_stream_t *(*connect_fn)(void *ctx);

// implementation methods
typedef struct {
    connect_fn connect_impl;
    destroy_fn destroy_ctx; // destructor for context, may be NULL
} _fh_connector_impl;

// instance data
struct _fh_connector_t {
    void *ctx;
    _fh_connector_impl *impl;
};

// connector implmentation contexts
typedef struct {
    char *ip;
    uint16_t port;
} _fh_tcp_client_ctx;

typedef struct {
    uint16_t port;
} _fh_tcp_server_ctx;

typedef struct {
    int fdin;
    int fdout;
} _fh_file_ctx;

// forwards
fh_stream_t * _fh_connector_connect_tcp(void *ctx);
void _fh_connector_destoy_ctx_tcp(void **ctx_p);

fh_stream_t * _fh_connector_connect_tcp_server(void *ctx);
void _fh_connector_destoy_ctx_tcp_server(void **ctx_p);

fh_stream_t * _fh_connector_connect_file(void *ctx);
void _fh_connector_destoy_ctx_file(void **ctx_p);


_fh_connector_impl TCP_CLIENT_CONNECTOR = {&_fh_connector_connect_tcp, &_fh_connector_destoy_ctx_tcp};
_fh_connector_impl TCP_SERVER_CONNECTOR = {&_fh_connector_connect_tcp_server, &_fh_connector_destoy_ctx_tcp_server};
_fh_connector_impl FILE_CONNECTOR = {&_fh_connector_connect_file, &_fh_connector_destoy_ctx_file};

// construct a connector
fh_connector_t *
fh_connector_new(void *context, _fh_connector_impl *impl)
{
    fh_connector_t *self = (fh_connector_t *)calloc(1, sizeof(fh_connector_t));
    assert(self);

    self->ctx = context;
    self->impl = impl;

    return self;
}

//  Destroy a connector
void
fh_connector_destroy(fh_connector_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_connector_t *self = *self_p;
        if (self->impl->destroy_ctx) {
            (*(self->impl->destroy_ctx))(&(self->ctx));
        }
        free(self);
        *self_p = NULL;
    }
}

//  Create a new tcp/ip client connector
fh_connector_t *
fh_connector_new_tcp_client(char *server_ip, uint16_t server_port)
{
    _fh_tcp_client_ctx *tcp_ctx = (_fh_tcp_client_ctx *)calloc(1, sizeof(_fh_tcp_client_ctx));
    assert(tcp_ctx);

    size_t ip_len = strlen(server_ip);
    tcp_ctx->ip = calloc(1, ip_len);
    strncpy(tcp_ctx->ip, server_ip, ip_len);

    tcp_ctx->port = server_port;

    return fh_connector_new(tcp_ctx, &TCP_CLIENT_CONNECTOR);
}

//  Create a new tcp/ip server connector
fh_connector_t *
fh_connector_new_tcp_server(uint16_t server_port)
{
    _fh_tcp_server_ctx *tcp_ctx = (_fh_tcp_server_ctx *)calloc(1, sizeof(_fh_tcp_server_ctx));
    assert(tcp_ctx);

    tcp_ctx->port = server_port;

    return fh_connector_new(tcp_ctx, &TCP_SERVER_CONNECTOR);
}

//  Create a new file connector
fh_connector_t *
fh_connector_new_file(int fdin, int fdout)
{
    _fh_file_ctx *file_ctx = (_fh_file_ctx *)calloc(1, sizeof(_fh_file_ctx));
    assert(file_ctx);

    file_ctx->fdin = fdin;
    file_ctx->fdout = fdout;

    return fh_connector_new(file_ctx, &FILE_CONNECTOR);
}

fh_stream_t *
fh_connector_connect(fh_connector_t *self)
{
    return (*(self->impl->connect_impl))(self->ctx);
}

// ###########################################################
// tcp client impl
// ###########################################################
fh_stream_t *
_fh_connector_connect_tcp(void *ctx)
{
    assert(ctx);
    _fh_tcp_client_ctx *narrow = (_fh_tcp_client_ctx *)ctx;

    int server_fd = fh_socket_util_connect_tcpip(narrow->ip, narrow->port);

    if (server_fd > 0) {
        // initialzetransport
        fh_stream_t *stream = fh_stream_new_socket(server_fd);
        return stream;
    }
    else {
        return NULL;
    }
}

void
_fh_connector_destoy_ctx_tcp(void **ctx_p)
{
    _fh_tcp_client_ctx *narrow = *(_fh_tcp_client_ctx **)ctx_p;
    free(narrow->ip);
    free(narrow);
}

// ###########################################################
// tcp server impl
// ###########################################################
fh_stream_t *
_fh_connector_connect_tcp_server(void *ctx)
{
    assert(ctx);
    _fh_tcp_server_ctx *narrow = (_fh_tcp_server_ctx *)ctx;

    // open socket/wait for client connection
    int ss_fd = fh_socket_util_open_server_socket(narrow->port);
    if (ss_fd < 1) {
        return NULL;
    }

    int client_fd = fh_socket_util_wait_for_connect(ss_fd);

    // char buf[INET_ADDRSTRLEN];
    // printf("Client connection from: %s %d\n", extract_remote_ip(client_fd, buf), extract_remote_port(client_fd));

    if (client_fd > 0) {
        // initialzetransport
        fh_stream_t *stream = fh_stream_new_socket(client_fd);
        return stream;
    }
    else {
        return NULL;
    }
}

void
_fh_connector_destoy_ctx_tcp_server(void **ctx_p)
{
    _fh_tcp_server_ctx *narrow = *(_fh_tcp_server_ctx **)ctx_p;
    free(narrow);
    *ctx_p = NULL;
}

// ###########################################################
// file impl
// ###########################################################
fh_stream_t *
_fh_connector_connect_file(void *ctx)
{
    assert(ctx);
    _fh_file_ctx *self = (_fh_file_ctx *)ctx;

        return fh_stream_new_file_desc(self->fdin, self->fdout);
}

void
_fh_connector_destoy_ctx_file(void **ctx_p)
{
    _fh_file_ctx *self = *(_fh_file_ctx **)ctx_p;
    free(self);
    *ctx_p = NULL;
}

