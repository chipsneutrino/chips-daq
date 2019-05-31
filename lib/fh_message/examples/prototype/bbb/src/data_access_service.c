/**
 * data_access_service.c
 *
 * Data Access service.
 *
 **/
#include <standard_inc.h>

#include "fh_library.h"

#include "data_access_service.h"

// forwards
static int _handle_poll_page(void *ctx, fh_message_t *msgin, fh_transport_t *transport);
static void _copy_page_to_msg(void *ctx, uint8_t *page, size_t length);

// instance data
struct _ds_service_t {
    fh_service_t *service; // holds typcode/subcode function bindings
    page_buffer pagebuf;   // page buffer
};

// create a new data access service
ds_service_t *
ds_new(uint8_t typecode, page_buffer *pagebuf)
{
    ds_service_t *self = (ds_service_t *)calloc(1, sizeof(ds_service_t));
    assert(self);

    fh_service_t *srvc = fh_service_new(typecode, self);
    fh_service_register_function(srvc, DS_POLL_PAGE, &_handle_poll_page);
    self->service = srvc;

    self->pagebuf.ctx = pagebuf->ctx;
    self->pagebuf.pop_page = pagebuf->pop_page;

    return self;
}

// destroy a data access service
void
ds_destroy(ds_service_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        ds_service_t *self = *self_p;

        fh_service_destroy(&(self->service)); // destroy service

        free(self);
        *self_p = NULL;
    }
}

// register the service with a message distpatcher
void
ds_register_service(ds_service_t *self, fh_dispatch_t *dispatcher)
{
    fh_dispatch_register_service(dispatcher, self->service);
}

// ##################################################################
// DATA_ACCESS_SERVICE functions
// ##################################################################
static int
_handle_poll_page(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    ds_service_t *self = (ds_service_t *)ctx;

    // bool page_available = hitbuf_pop_page_to(self->hitbuf, &(page_sink){msgin, &_copy_page_to_msg});
    bool page_available =
        (*(self->pagebuf.pop_page))(self->pagebuf.ctx, &(page_sink){.ctx = msgin, .callback = &_copy_page_to_msg});
    if (page_available) {
        // page content was copied into message by _copy_page_to_msg()
    }
    else {
        // _copy_page_to_msg() was no called, send an empy page sentinel
        fh_message_setData(msgin, NULL, 0);
    }
    return fh_transport_send(transport, msgin);
}

// ##################################################################
// private functions
// ##################################################################

// page sink function
void
_copy_page_to_msg(void *ctx, uint8_t *page, size_t length)
{
    fh_message_t *msg = (fh_message_t *)ctx;
    fh_message_setData(msg, page, length);
}
