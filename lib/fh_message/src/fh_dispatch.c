/**
 * fh_dispatch.c
 *
 * Provides dispatching services for incoming messages.
 */

#include "fh_classes.h"

// forwards
static int
_fh_dispatch_no_service_handler(fh_message_t *msgin, fh_transport_t *transport);

// instance data
struct _fh_dispatch_t {
    fh_service_t *services[256];
};

//  Create a new dispatch
fh_dispatch_t *
fh_dispatch_new(void)
{
    fh_dispatch_t *self = (fh_dispatch_t *)calloc(1,sizeof(fh_dispatch_t));
    assert(self);

    for (int i = 0; i < 256; i++) {
        self->services[i] = NULL;
    }
    return self;
}

//  Destroy the dispatch
void
fh_dispatch_destroy(fh_dispatch_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_dispatch_t *self = *self_p;
        free(self);
        *self_p = NULL;
    }
}

// register a service
void
fh_dispatch_register_service(fh_dispatch_t *self, fh_service_t *service)
{
    uint8_t tc = fh_service_getTypecode(service);
    assert(tc > FH_ERROR_SERVICE_CODE);
    self->services[tc] = service;
}

// dispatch a message to the appropriate service
int
fh_dispatch_handle(fh_dispatch_t *self, fh_message_t *msgin, fh_transport_t *transport)
{
    uint8_t type = fh_message_getType(msgin);

    if (self->services[type]) {
        return fh_service_handle(self->services[type], msgin, transport);
    }
    else {
        return _fh_dispatch_no_service_handler(msgin, transport);
    }
}

// handler for messages with no service registered.
static int
_fh_dispatch_no_service_handler(fh_message_t *msgin, fh_transport_t *transport)
{
    return err_invalid_cmd(msgin, transport);
}
