/**
 * fh_dispatch.h
 *
 * Provides dispatching services for incoming messages.
 */

#ifndef FH_DISPATCH_H_INCLUDED
#define FH_DISPATCH_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// service code reserved for error messages
#define FH_ERROR_SERVICE_CODE 0

//  Create a new dispatch
fh_dispatch_t *
fh_dispatch_new(void);

//  Destroy the dispatch
void
fh_dispatch_destroy(fh_dispatch_t **self_p);

// register a service
void
fh_dispatch_register_service(fh_dispatch_t *self, fh_service_t *service);

// dispatch a message to the appropriate service
int
fh_dispatch_handle(fh_dispatch_t *self, fh_message_t *msgin, fh_transport_t *transport);

#ifdef __cplusplus
}
#endif

#endif
