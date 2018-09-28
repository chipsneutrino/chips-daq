/**
 * fh_comp.c
 *
 * Controls a single field hub on a dedicated thread.
 */
 #include "standard_inc.h"

#include "fh_comp.h"

 // forwards
static bool _init_action(void *ctx);
static bool _config_action(void *ctx);
static bool _start_run_action(void *ctx);
static bool _running_action(void *ctx, bool *yield);
static bool _stop_run_action(void *ctx);

// instance data
struct _fh_comp_t {
    page_handler page_handler; // callback for acquire pages
    fh_peer_t *fh_peer;        // local interface to a remote field hub
    uint32_t fh_id;            // unique identifier of attached field hub
};

// Create a new field hub comp
fh_comp_t *
fh_comp_new(fh_peer_t *fh_peer, page_handler page_handler)
{
    fh_comp_t *self = (fh_comp_t *)calloc(1, sizeof(fh_comp_t));
    assert(self);

    self->page_handler = page_handler;
    self->fh_peer = fh_peer;
    self->fh_id = fh_peer_get_id(fh_peer);

    return self;
}

// Destroy the field hub controller
void
fh_comp_destroy(fh_comp_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_comp_t *self = *self_p;

        fh_peer_destroy(&(self->fh_peer)); // destroy peer

        free(self);
        *self_p = NULL;
    }
}

// extract the state_component_t interface
void
fh_comp_as_state_comp(fh_comp_t *self, state_component_t *comp)
{
    comp->ctx = self;
    comp->init_action = &_init_action;
    comp->config_action = &_config_action;
    comp->start_run_action = &_start_run_action;
    comp->running_action = &_running_action;
    comp->stop_run_action = &_stop_run_action;
}

static bool
_init_action(void *ctx)
{
    fh_comp_t *self = (fh_comp_t *)ctx;
    if (fh_peer_get_state(self->fh_peer) == DISCONNECTED) {
        return fh_peer_connect(self->fh_peer);
    }
    else {
        return false;
    }
}

static bool
_config_action(void *ctx)
{
    fh_comp_t *self = (fh_comp_t *)ctx;
    if (fh_peer_get_state(self->fh_peer) == CONNECTED) {
        return fh_peer_configure(self->fh_peer);
    }
    else {
        return false;
    }
}

static bool
_start_run_action(void *ctx)
{
    fh_comp_t *self = (fh_comp_t *)ctx;
    if (fh_peer_get_state(self->fh_peer) == CONNECTED) {
        return fh_peer_start_run(self->fh_peer);
    }
    else {
        return false;
    }
}

static bool
_running_action(void *ctx, bool *yield)
{
    fh_comp_t *self = (fh_comp_t *)ctx;

    if (fh_peer_get_state(self->fh_peer) == CONNECTED) {
        size_t length = -1;
        bool status = fh_peer_poll_page(self->fh_peer, &(self->page_handler), &length);

        if (length < 1) {
            // send a squelch signal back to controller
            *yield = true;
        }
        else {
            *yield = false;
        }

        return status;
    }
    else {
        return false;
    }
}

static bool
_stop_run_action(void *ctx)
{
    fh_comp_t *self = (fh_comp_t *)ctx;
    if (fh_peer_get_state(self->fh_peer) == CONNECTED) {
        return fh_peer_stop_run(self->fh_peer);
    }
    else {
        return false;
    }
}
