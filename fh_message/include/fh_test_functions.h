/**
 * fh_test_methods.h
 *
 * Exports public test functions.
 *
 * Certain test functions are exported to support execution on the bare-metal
 * microdaq platform. 
 */

#ifndef FH_TEST_FUNCTIONS_H_INCLUDED
#define FH_TEST_FUNCTIONS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


// Entry point for the server side of the transport test
void * fh_transport_test_server_th(fh_transport_t *transport, FILE *log);

// entry point for the client  of the transport test
void * fh_transport_test_client_th(fh_transport_t *transport, FILE *log);

//  Run all self tests.
void fh_test_runall (bool verbose);


#ifdef __cplusplus
}
#endif

#endif
