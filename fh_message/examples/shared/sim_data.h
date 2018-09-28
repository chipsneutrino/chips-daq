/**
 * sim_data.h
 *
 * Simulate a microdaq source with an asynch producer pushing simulated hits
 * into a hit buffer.
 * 
 */
#ifndef SIM_DATA_INCLUDED
#define SIM_DATA_INCLUDED

#include "hitbuf.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sim_data_t sim_data_t;

typedef struct {      
    int rps;     // hit rate in records/second
} sim_spec;

//  Create a new sim data
sim_data_t *
sim_data_new(hitbuf_t *hit_buffer, sim_spec spec);

//  Destroy the sim data
void
sim_data_destroy(sim_data_t **self_p);

//  launches the data prodcution thread
bool
sim_data_start(sim_data_t *self);

//  stops the data prodcution thread
bool
sim_data_stop(sim_data_t *self);


#ifdef __cplusplus
}
#endif

#endif
