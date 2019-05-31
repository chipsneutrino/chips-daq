/**
 * fh_service.c
 *
 * A skeleton service implementation.
 *
 */

#include "fh_classes.h"

// forwards
static int
_fh_service_no_function_handler(void *ctx, fh_message_t *msgin, fh_transport_t *transport);

// instance data
struct _fh_service_t {
    uint8_t typecode;
    service_fn functions[256];
    void *context;
};

// create a new service instance. Client retains ownership of context memory.
fh_service_t *
fh_service_new(uint8_t typecode, void *context)
{
    fh_service_t *self = (fh_service_t *)calloc(1,sizeof(fh_service_t));
    assert(self);

    self->typecode = typecode;
    for (int i = 0; i < 256; i++) {
        self->functions[i] = &_fh_service_no_function_handler;
    }

    self->context = context;
    return self;
}

// destroy a service instance
void
fh_service_destroy(fh_service_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_service_t *self = *self_p;
        free(self);
        *self_p = NULL;
    }
}

uint8_t
fh_service_getTypecode(fh_service_t *self)
{
    return self->typecode;
}

void
fh_service_register_function(fh_service_t *self, uint8_t subcode, service_fn service)
{
    self->functions[subcode] = service;
}

int
fh_service_handle(fh_service_t *self, fh_message_t *msgin, fh_transport_t *transport)
{
    assert(fh_message_getType(msgin) == self->typecode);
    uint8_t subtype = fh_message_getSubtype(msgin);

    return (*(self->functions[subtype]))(self->context, msgin, transport);
}

static int
_fh_service_no_function_handler(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    uint8_t type = fh_message_getType(msgin);
    uint8_t st = fh_message_getSubtype(msgin);
    fh_message_init_ascii_msg(msgin, "No function for service type [%d, %d]", type,st);

    return fh_transport_send(transport, msgin);
}
