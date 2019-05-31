/**
 * data_access_service.h
 *
 * Data Access service.
 *
 **/

#ifndef _DATA_ACCESS_SERVICE_H_
#define _DATA_ACCESS_SERVICE_H_

#include "fh_library.h"
#include "data_access_service_api.h"
#include "hitbuf.h"

typedef struct _ds_service_t ds_service_t;

// interface to page buffer
typedef bool (*pop_page_to_fn)(void *ctx, page_sink *sink);
typedef struct {
    void *ctx;
    pop_page_to_fn pop_page;
} page_buffer;

// create a new data access service
ds_service_t *
ds_new(uint8_t typecode, page_buffer *pagebuf);

// destroy a data access service
void
ds_destroy(ds_service_t **self_p);

// register the service with a message distpatcher
void
ds_register_service(ds_service_t *self, fh_dispatch_t *dispatcher);

#endif
