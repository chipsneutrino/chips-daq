/**
 * fh_comp.h
 *
 * Controls a single field hub on a dedicated thread.
 */
#include <stdbool.h>

 #include "fh_peer.h"
 #include "state_ctrl.h"


#ifndef FH_COMP_INCLUDED
#define FH_COMP_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fh_comp_t fh_comp_t;

// Create a new field hub comp
fh_comp_t *
fh_comp_new(fh_peer_t *fh_peer, page_handler page_handler);

// Destroy the field hub controller
void
fh_comp_destroy(fh_comp_t **self_p);

// extract the state_component_t interface
void
fh_comp_as_state_comp(fh_comp_t *self, state_component_t *comp);

#ifdef __cplusplus
}
#endif

#endif
