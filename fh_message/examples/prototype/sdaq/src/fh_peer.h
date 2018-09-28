/**
 * fh_peer.h
 *
 * provide a local interface to a remote field hub.
 */

#include "fh_library.h"
#include "fh_connector.h"


#ifndef FH_PEER_INCLUDED
#define FH_PEER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fh_peer_t fh_peer_t;

typedef enum { DISCONNECTED, CONNECTED } peer_state;

// page storage callback
typedef void (*handle_page_fn)(void *ctx, uint8_t fh_id, uint8_t *page, size_t len);
typedef struct {
    void *ctx;
    handle_page_fn handle_page;
} page_handler;


fh_peer_t*
fh_peer_new(uint32_t fh_id, fh_connector_t *connector);

void
fh_peer_destroy(fh_peer_t **self_p);

void
fh_peer_trace_protocol(fh_peer_t *self, FILE *out);

void
fh_peer_trace_stream(fh_peer_t *self, FILE *out);

uint32_t
fh_peer_get_id(fh_peer_t *self);

peer_state
fh_peer_get_state(fh_peer_t *self);

bool
fh_peer_connect(fh_peer_t *self);

bool
fh_peer_configure(fh_peer_t *self);

bool
fh_peer_start_run(fh_peer_t *self);

  bool
fh_peer_stop_run(fh_peer_t *self);

bool
fh_peer_poll_page(fh_peer_t *self, page_handler *handler, size_t *length);


#ifdef __cplusplus
}
#endif

#endif
