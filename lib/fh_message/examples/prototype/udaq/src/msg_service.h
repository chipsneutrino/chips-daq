/**
 * msg_service.h
 *
 * Message service.
 *
 **/

#ifndef _MSG_SERVICE_H_
#define _MSG_SERVICE_H_

#include "fh_library.h"
#include "msg_service_api.h"

typedef struct _ms_service_t ms_service_t;

// connection management callback provided by client
typedef void (*close_fn)();

// create a new message service
ms_service_t *
ms_new(uint8_t typecode, close_fn close);

// destroy a message service
void
ms_destroy(ms_service_t **self_p);

// register the service with a message distpatcher
void
ms_register_service(ms_service_t *self, fh_dispatch_t *dispatcher);

#endif
