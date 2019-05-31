/**
 * sim_udaq.h
 *
 * Simulate a microdaq.
 */
#include <stdbool.h>

#include "fh_library.h"
#include "fh_connector.h"


#ifndef SIM_UDAQ_INCLUDED
#define SIM_UDAQ_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sim_udaq_t sim_udaq_t;

typedef struct {
    uint64_t device_id; // device id
    uint8_t buffer_sz;  // buffer size (bits)
    uint8_t page_sz;    // buffer page size (bits)
    uint32_t hit_rate;  // hit rate (Hz)
} sim_udaq_spec;

// Create a new sim udaq
sim_udaq_t *
sim_udaq_new(sim_udaq_spec spec, fh_connector_t *hub_connector);

// Destroy the sim udaq
void
sim_udaq_destroy(sim_udaq_t **self_p);

// launches main thread
bool
sim_udaq_start(sim_udaq_t *self);

// connect a client control channel
void
sim_udaq_connect(sim_udaq_t *self, fh_transport_t *transport);

// block waiting for udaq main loop to exit.
void
sim_udaq_join(sim_udaq_t *self);

#ifdef __cplusplus
}
#endif

#endif
