/**
 * fh_transport_test.h
 *
 * Entry points for a client/server pair that test a message transport
 * implementation in the face of bit errors in the connection.
 *
 */

#ifndef FH_TRANSPORT_TEST_H_INCLUDED
#define FH_TRANSPORT_TEST_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// Entry point for the server
void * fh_transport_test_server_th(fh_transport_t *transport, FILE *log);

// entry point for the client
void * fh_transport_test_client_th(fh_transport_t *transport, FILE *log);


//  Run all self tests
void
fh_selftest_runall (bool verbose);


#ifdef __cplusplus
}
#endif

#endif
