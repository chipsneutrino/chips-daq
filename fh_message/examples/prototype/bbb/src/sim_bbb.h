/**
 * sim_bbb.h
 *
 * Simulate a beagle bone field hub.
 */
#include <stdbool.h>

#include "fh_connector.h"
#include "fh_library.h"
#include "udaq_peer.h"

#ifndef SIM_BBB_INCLUDED
#define SIM_BBB_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sim_bbb_t sim_bbb_t;

// configuration shim
extern void configure_udaqs(sim_bbb_t *bbb);

// Create a new sim bbb
sim_bbb_t *
sim_bbb_new(fh_connector_t *sdaq_connector, uint8_t buffer_sz, uint8_t page_sz);

// Destroy the sim bbb
void
sim_bbb_destroy(sim_bbb_t **self_p);

// launches threads
bool
sim_bbb_start(sim_bbb_t *self);

// block waiting for bbb main loop to exit.
void
sim_bbb_join(sim_bbb_t *self);

// add a udaq
void
sim_bbb_configure_udaq(sim_bbb_t *self, udaq_peer_t *udaq, uint8_t thread_group);

// support in-process exp control
void
sim_bbb_configure(sim_bbb_t *self);

// support in-process exp control
void
sim_bbb_start_run(sim_bbb_t *self);

// support in-process exp control
void
sim_bbb_stop_run(sim_bbb_t *self);

// enable tracing of messages
void
sim_bbb_trace_protocol(sim_bbb_t *self, FILE *out);

// enable tracing of stream operations
void
sim_bbb_trace_stream(sim_bbb_t *self, FILE *out);



#ifdef __cplusplus
}
#endif

#endif
