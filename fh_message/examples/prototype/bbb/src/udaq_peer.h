/**
 * udaq_peer.h
 *
 * Provides a local interface to a remote udaq.
 */
#include <stdbool.h>

#include "fh_library.h"
 #include "fh_connector.h"


#ifndef UDAQ_PEER_INCLUDED
#define UDAQ_PEER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _udaq_peer_t udaq_peer_t;

typedef enum { DISCONNECTED, CONNECTED } peer_state;

// page hadler callback
typedef void (*handle_page_fn)(void *ctx, uint8_t channel_id, uint8_t *page, size_t len);
typedef struct {
    void *ctx;
    handle_page_fn handle_page;
} page_handler;

udaq_peer_t*
udaq_peer_new(uint8_t channel_id, fh_connector_t *connector);

void
udaq_peer_destroy(udaq_peer_t **self_p);

void
udaq_peer_trace_protocol(udaq_peer_t *self, FILE *out);

peer_state
udaq_peer_get_state(udaq_peer_t *self);

bool
udaq_peer_connect(udaq_peer_t *self);

bool
udaq_peer_start_run(udaq_peer_t *self);

bool
udaq_peer_stop_run(udaq_peer_t *self);

bool
udaq_peer_poll_page(udaq_peer_t *self, page_handler *handler, size_t *length);



#ifdef __cplusplus
}
#endif

#endif
