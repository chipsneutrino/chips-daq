/**
 * udaq_comp.h
 *
 * Performs state-based actions performed against a udaq instance.
 */
#include <stdbool.h>

#include "fh_library.h"
 #include "fh_connector.h"
 #include "udaq_peer.h"


#ifndef UDAQ_COMPONENT_INCLUDED
#define UDAQ_COMPONENT_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _udaq_comp_t udaq_comp_t;

// create a new udaq component. Component takes ownership of udaq_peer.
udaq_comp_t *
udaq_comp_new(udaq_peer_t *udaq_peer, page_handler page_handler);

// destroy a udaq component
void
udaq_comp_destroy(udaq_comp_t **self_p);

// invokes by the state controller during initilaization
bool
udaq_comp_init_action(void *ctx);

// invokes by the state controller during configuration
bool
udaq_comp_config_action(void *ctx);

// invokes by the state controller during run start
bool
udaq_comp_start_run_action(void *ctx);

// invokes by the state controller repeatedly while running
bool
udaq_comp_running_action(void *ctx, bool *yield);

// invoked by the state controller during run stop
bool
udaq_comp_stop_run_action(void *ctx);

#ifdef __cplusplus
}
#endif

#endif
