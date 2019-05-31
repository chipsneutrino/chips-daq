/**
 * udaq_comp.c
 *
 * Performs state-based actions performed against a udaq instance.
 */
#include "standard_inc.h"

#include "udaq_comp.h"
#include "udaq_peer.h"

// instance data
struct _udaq_comp_t {
    udaq_peer_t *udaq_peer;    // udaq interface.
    page_handler page_handler; // callback target for aquired data pages
};

// create a new udaq component. Component takes ownership of udaq_peer.
udaq_comp_t *
udaq_comp_new(udaq_peer_t *udaq_peer, page_handler page_handler)
{
    udaq_comp_t *self = (udaq_comp_t *)calloc(1, sizeof(udaq_comp_t));
    assert(self);

    self->udaq_peer = udaq_peer;

    self->page_handler.ctx = page_handler.ctx;
    self->page_handler.handle_page = page_handler.handle_page;

    return self;
}

// destroy a udaq component
void
udaq_comp_destroy(udaq_comp_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        udaq_comp_t *self = *self_p;

        udaq_peer_destroy(&(self->udaq_peer));

        free(self);
        *self_p = NULL;
    }
}

// invokes by the state controller during initilaization
bool
udaq_comp_init_action(void *ctx)
{
    udaq_comp_t *self = (udaq_comp_t *)ctx;
    if (udaq_peer_get_state(self->udaq_peer) == DISCONNECTED) {
        return udaq_peer_connect(self->udaq_peer);
    }
    else {
        return false;
    }
}

// invokes by the state controller during configuration
bool
udaq_comp_config_action(void *ctx)
{
    udaq_comp_t *self = (udaq_comp_t *)ctx;
    if (udaq_peer_get_state(self->udaq_peer) == CONNECTED) {
        printf("todo: send config parameters to udaq\n");
        return true;
    }
    return false;
}

// invokes by the state controller during run start
bool
udaq_comp_start_run_action(void *ctx)
{
    udaq_comp_t *self = (udaq_comp_t *)ctx;
    if (udaq_peer_get_state(self->udaq_peer) == CONNECTED) {
        return udaq_peer_start_run(self->udaq_peer);
    }
    else {
        return false;
    }
}

// invokes by the state controller repeatedly while running
bool
udaq_comp_running_action(void *ctx, bool *yield)
{
    udaq_comp_t *self = (udaq_comp_t *)ctx;
    if (udaq_peer_get_state(self->udaq_peer) == CONNECTED) {
        size_t length = -1;
        bool status = udaq_peer_poll_page(self->udaq_peer, &(self->page_handler), &length);

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

// invoked by the state controller during run stop
bool
udaq_comp_stop_run_action(void *ctx)
{
    udaq_comp_t *self = (udaq_comp_t *)ctx;
    if (udaq_peer_get_state(self->udaq_peer) == CONNECTED) {
        return udaq_peer_stop_run(self->udaq_peer);
    }
    else {
        return false;
    }
}
