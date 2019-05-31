/**
 * msg_service.c
 *
 * Message service.
 *
 **/

#include <assert.h>
#include <string.h>

#include "fh_library.h"

#include "msg_service.h"
// ##################################################################
// MESSAGE_SERVER functions
// ##################################################################
// The msg server implements the following commands:
//
// [subcode] <function desc>
// payload in:
// payload out:
//
// [1] <status>
// in: n/a
// out: uint8 : 1=ready, 2=fault
//
// [2] <echo>
// in: uint8[]  : data to echo
// out: uint8[] : copy of in
//
// [3] <close>
// in: n/a
// out: uint_8[] : "GOODBYE"
//
//
// ------------------------------------------------------------------




//forwards
int ms_status(void *ctx, fh_message_t *msgin, fh_transport_t *transport);
int ms_echo(void *ctx, fh_message_t *msgin, fh_transport_t *transport);
int ms_close(void *ctx, fh_message_t *msgin, fh_transport_t *transport);

// instance data
struct _ms_service_t {
    fh_service_t *service; // holds typcode/subcode function bindings
    close_fn close;        // callback to close message channel
};

// create a new message service
ms_service_t *
ms_new(uint8_t typecode, close_fn close)
{
    ms_service_t *self = (ms_service_t *)calloc(1, sizeof(ms_service_t));
    assert(self);

    fh_service_t *srvc = fh_service_new(typecode, self);       // new service with user-provided typecode
    fh_service_register_function(srvc, MS_STATUS, &ms_status); // bind "status" function to subcode MS_STATUS
    fh_service_register_function(srvc, MS_ECHO, &ms_echo);     // bind "echo" functionto subcode MS_ECHO
    fh_service_register_function(srvc, MS_CLOSE, &ms_close);   // bind "close" functionto subcode MS_CLOSE
    self->service = srvc;

    self->close = close;
    return self;
}

// destroy a message service
void
ms_destroy(ms_service_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        ms_service_t *self = *self_p;

        fh_service_destroy(&(self->service)); // destroy service

        free(self);
        *self_p = NULL;
    }
}

// register the service with a message distpatcher
void
ms_register_service(ms_service_t *self, fh_dispatch_t *dispatcher)
{
    fh_dispatch_register_service(dispatcher, self->service);
}

// ##################################################################
// MESSAGE_SERVER functions
// ##################################################################
int
ms_status(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    uint8_t out[] = {1}; // OK
    fh_message_setData(msgin, out, 1);
    return fh_transport_send(transport, msgin);
}

int
ms_echo(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    int size = fh_message_dataLen(msgin);
    uint8_t *original = fh_message_getData(msgin);
    uint8_t copy[size];
    memcpy(copy, original, size);
    fh_message_setData(msgin, copy, size);
    return fh_transport_send(transport, msgin);
}

int
ms_close(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    ms_service_t *self = (ms_service_t *)ctx;

    fh_message_setData(msgin, (uint8_t *)"GOODBYE", 7);
    int status = fh_transport_send(transport, msgin);

    (*(self->close))();

    return status;
}
