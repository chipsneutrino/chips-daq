/**
 * sim_sdaq.h
 *
 * Simulate an sdaq.
 */
#include <stdbool.h>

#include "fh_library.h"
 #include "fh_connector.h"
 #include "fh_peer.h"


#ifndef SIM_SDAQ_INCLUDED
#define SIM_SDAQ_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sim_sdaq_t sim_sdaq_t;

// configuration shim
extern void configure_field_hubs(sim_sdaq_t *bbb);

// Create a new sim sdaq
sim_sdaq_t *
sim_sdaq_new();

// Destroy the sim sdaq
void
sim_sdaq_destroy(sim_sdaq_t **self_p);

// launches main thread
bool
sim_sdaq_start(sim_sdaq_t *self);

// block waiting for main loop to exit.
void
sim_sdaq_join(sim_sdaq_t *self);

// add a field hub peer
void
sim_sdaq_configure_fh(sim_sdaq_t *self, fh_peer_t *bbb);

void
sim_sdaq_configure(sim_sdaq_t *self);

void
sim_sdaq_start_run(sim_sdaq_t *self);

void
sim_sdaq_stop_run(sim_sdaq_t *self);


#ifdef __cplusplus
}
#endif

#endif
