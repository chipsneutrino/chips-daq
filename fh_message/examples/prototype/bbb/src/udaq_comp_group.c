/**
 * udaq_comp_group.c
 *
 * Composes a set of udaq components into a singe unit. State actions
 * are then serialized on a single state controller thread.
 */
#include "standard_inc.h"

#include "state_ctrl.h"
#include "udaq_comp.h"
#include "udaq_comp_group.h"

// tracks status of a udaq_comp
typedef enum { OK, FAULT } udaq_status;

// holds a udaq component with error flag
typedef struct {
    udaq_comp_t *udaq_comp;
    udaq_status status;
} udaq_t;

// instance data
struct _udaq_comp_group_t {
    size_t max_comp; // maximum number of components
    udaq_t **udaq_v; // storage for udaq_t pointers.
    size_t num_comp; // number of components
};

udaq_comp_group_t *
udaq_comp_group_new(size_t max_comp)
{
    udaq_comp_group_t *self = (udaq_comp_group_t *)calloc(1, sizeof(udaq_comp_group_t));
    assert(self);

    self->max_comp = max_comp;
    self->num_comp = 0;
    self->udaq_v = (udaq_t **)calloc(1, sizeof(udaq_t *) * self->max_comp);
    assert(self->udaq_v);

    return self;
}

void
udaq_comp_group_destroy(udaq_comp_group_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        udaq_comp_group_t *self = *self_p;

        // dynamically allocated
        for (int i = 0; i < self->num_comp; i++) {
            udaq_comp_destroy(&(self->udaq_v[i]->udaq_comp));
            free(self->udaq_v[i]);
        }
        free(self->udaq_v);

        free(self);
        *self_p = NULL;
    }
}

void
udaq_comp_group_add_udaq(udaq_comp_group_t *self, udaq_comp_t *udaq_comp)
{
    assert(self->num_comp < self->max_comp);
    udaq_t *udaq = (udaq_t *)calloc(1, sizeof(udaq_t));
    assert(udaq);

    udaq->udaq_comp = udaq_comp;
    udaq->status = OK;

    self->udaq_v[self->num_comp] = udaq;
    self->num_comp++;
}

// invoked by the state controller during initilaization
static bool
_init_action(void *ctx)
{
    udaq_comp_group_t *self = (udaq_comp_group_t *)ctx;

    for (int i = 0; i < self->num_comp; i++) {
        if (self->udaq_v[i]->status == OK) {
            bool success = udaq_comp_init_action(self->udaq_v[i]->udaq_comp);
            if (!success) {
                printf("Dropping udaq %d\n", i);
                self->udaq_v[i]->status = FAULT; // dropped udaq
            }
        }
    }
    return true;
}

// invoked by the state controller during configuration
static bool
_config_action(void *ctx)
{
    udaq_comp_group_t *self = (udaq_comp_group_t *)ctx;

    for (int i = 0; i < self->num_comp; i++) {
        if (self->udaq_v[i]->status == OK) {
            bool success = udaq_comp_config_action(self->udaq_v[i]->udaq_comp);
            if (!success) {
                printf("Dropping udaq %d\n", i);
                self->udaq_v[i]->status = FAULT; // dropped udaq
            }
        }
    }
    return true;
}

// invoked by the state controller during run start
static bool
_start_run_action(void *ctx)
{
    udaq_comp_group_t *self = (udaq_comp_group_t *)ctx;

    for (int i = 0; i < self->num_comp; i++) {
        if (self->udaq_v[i]->status == OK) {
            bool success = udaq_comp_start_run_action(self->udaq_v[i]->udaq_comp);
            if (!success) {
                printf("Dropping udaq %d\n", i);
                self->udaq_v[i]->status = FAULT; // dropped udaq
            }
        }
    }
    return true;
}

// invoked by the state controller repeatedly while running
static bool
_running_action(void *ctx, bool *yield)
{
    udaq_comp_group_t *self = (udaq_comp_group_t *)ctx;

    *yield = true;
    bool vote = true;
    for (int i = 0; i < self->num_comp; i++) {
        if (self->udaq_v[i]->status == OK) {
            bool success = udaq_comp_running_action(self->udaq_v[i]->udaq_comp, &vote);
            if (!success) {
                printf("Dropping udaq %d\n", i);
                self->udaq_v[i]->status = FAULT; // dropped udaq
            }
            else {
                *yield = *yield & vote; // only squelch controller if all udaqs are drained
            }
        }
    }
    return true;
}

// invoked by the state controller during run stop
static bool
_stop_run_action(void *ctx)
{
    udaq_comp_group_t *self = (udaq_comp_group_t *)ctx;

    for (int i = 0; i < self->num_comp; i++) {
        if (self->udaq_v[i]->status == OK) {
            bool success = udaq_comp_stop_run_action(self->udaq_v[i]->udaq_comp);
            if (!success) {
                printf("Dropping udaq %d\n", i);
                self->udaq_v[i]->status = FAULT; // dropped udaq
            }
        }
    }
    return true;
}

// extract the state_component_t interface
void
udaq_comp_group_as_state_comp(udaq_comp_group_t *self, state_component_t *comp)
{
    comp->ctx = self;
    comp->init_action = &_init_action;
    comp->config_action = &_config_action;
    comp->start_run_action = &_start_run_action;
    comp->running_action = &_running_action;
    comp->stop_run_action = &_stop_run_action;
}
