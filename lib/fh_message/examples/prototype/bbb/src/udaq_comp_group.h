/**
 * udaq_comp_group.h
 *
 * Composes a set of udaq components into a singe unit. State actions
 * are then serialized on a single state controller thread.
 */

#include "state_ctrl.h"
#include "udaq_comp.h"


#ifndef UDAQ_COMP_GROUP_INCLUDED
#define UDAQ_COMP_GROUP_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _udaq_comp_group_t udaq_comp_group_t;

udaq_comp_group_t *
udaq_comp_group_new(size_t max_comp);

void
udaq_comp_group_destroy(udaq_comp_group_t **self_p);

// add a udaq component.  Ownership of the component transfers to
// the udaq_comp_group instance.
void
udaq_comp_group_add_udaq(udaq_comp_group_t *self, udaq_comp_t *udaq_comp);

// extract the state_component_t interface
void
udaq_comp_group_as_state_comp(udaq_comp_group_t *self, state_component_t *comp);

#ifdef __cplusplus
}
#endif

#endif
