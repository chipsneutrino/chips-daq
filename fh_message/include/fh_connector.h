/**
 * fh_connector.h
 *
 * Encapsulates establishing a connection.
 *
 */
 

#ifndef FH_CONNECTOR_H_INCLUDED
#define FH_CONNECTOR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


//  Destroy a connector
void
fh_connector_destroy(fh_connector_t **self_p);

//  Create a new tcp/ip client connector
fh_connector_t *
fh_connector_new_tcp_client(char* server_ip, uint16_t server_port);

//  Create a new tcp/ip server connector
fh_connector_t *
fh_connector_new_tcp_server(uint16_t server_port);

//  Create a new file connector
fh_connector_t *
fh_connector_new_file(int fdin, int fdout);

// establish the connection
fh_stream_t*
fh_connector_connect(fh_connector_t *self);


#ifdef __cplusplus
}
#endif

#endif
