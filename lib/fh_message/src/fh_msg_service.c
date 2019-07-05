/**
 * fh_message_service.c
 *
 */

#include "fh_classes.h"

// service forwards
static int mgs_status(void *ctx, fh_message_t *msgin, fh_transport_t *transport);
static int mgs_echo(void *ctx, fh_message_t *msgin, fh_transport_t *transport);

// instance data
struct _fh_msg_service_t {
    fh_service_t *service; // holds typcode/subcode function bindings
};

//  Create a new msg_service
fh_msg_service_t *
fh_msg_service_new(uint8_t typecode)
{
    fh_msg_service_t *self = (fh_msg_service_t *)calloc(1,sizeof(fh_msg_service_t));
    assert(self);

    fh_service_t *srvc = fh_service_new(typecode, self);
    fh_service_register_function(srvc, MS_STATUS, &mgs_status);
    fh_service_register_function(srvc, MS_ECHO, &mgs_echo);
    self->service = srvc;

    return self;
}

//  Destroy the msg_service
void
fh_msg_service_destroy(fh_msg_service_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_msg_service_t *self = *self_p;
       
        fh_service_destroy(&(self->service)); // destroy service

        free(self);
        *self_p = NULL;
    }
}

// Register the service with a message distpatcher
void
fh_msg_service_register(fh_msg_service_t *self, fh_dispatch_t *dispatcher)
{
    fh_dispatch_register_service(dispatcher, self->service);
}

static int
mgs_status(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    uint8_t status[] = {1};
    fh_message_setData(msgin, status, 1);
    return fh_transport_send(transport, msgin);
}

static int
mgs_echo(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    return fh_transport_send(transport, msgin);
}

