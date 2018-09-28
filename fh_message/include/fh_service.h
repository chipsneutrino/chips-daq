/**
 * fh_service.h
 *
 * A skeleton service implementation.
 *
 */

#ifndef FH_SERVICE_H_INCLUDED
#define FH_SERVICE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// function pointer typedef for service function imlementations
typedef int (*service_fn)(void *ctx, fh_message_t *msgin, fh_transport_t *transport);

//  Create a new service. Client retains ownership of context memory.
fh_service_t *
fh_service_new(uint8_t typecode, void *context);

//  Destroy the service
void
fh_service_destroy(fh_service_t **self_p);

// the service type code
uint8_t
fh_service_getTypecode(fh_service_t *self);

// register a handler for a subtype function
void
fh_service_register_function(fh_service_t *self, uint8_t subcode, service_fn function);

// dispatch a message to the appropriate subtype function
int
fh_service_handle(fh_service_t *self, fh_message_t *msgin, fh_transport_t *transport);

#ifdef __cplusplus
}
#endif

#endif
