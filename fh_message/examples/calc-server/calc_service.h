/**
 * calc_service.h
 *
 * Calculator service.
 *
 **/

#ifndef _CALC_SERVICE_H_
#define _CALC_SERVICE_H_

#include "fh_library.h"
#include "calc_api.h"

typedef struct _calc_service_t calc_service_t;

// create a new calc service
calc_service_t *
calc_new(uint8_t typecode);

//  Destroy the calc service
void
calc_destroy(calc_service_t **self_p);

// register the service with a message distpatcher
void
calc_register_service(calc_service_t *self, fh_dispatch_t *dispatcher);

int
calc_add(void *ctx, fh_message_t *msg, fh_transport_t *transport);

int
calc_subtract(void *ctx, fh_message_t *msg, fh_transport_t *transport);

int
calc_multiply(void *ctx, fh_message_t *msg, fh_transport_t *transport);

int
calc_divide(void *ctx, fh_message_t *msg, fh_transport_t *transport);

#endif
