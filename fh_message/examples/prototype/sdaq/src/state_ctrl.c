/**
 * state_ctrl.c
 *
 * Manages state transitions of a component.
 */
#include "standard_inc.h"
#include <pthread.h>

#include "state_ctrl.h"

// forwards
static void _lock(state_ctrl_t *self);
static void _unlock(state_ctrl_t *self);
static void _state_ctrl_set_state(state_ctrl_t *self, state state);
static void * _state_ctrl_main(void *arg);

// instance data
struct _state_ctrl_t {
    state state;                 // current state
    pthread_t ctrl_th;           // controller thread
    pthread_mutex_t lock;        // pthread mutex
    state_component_t component; // component under control
};

// models an asyncronous state transition
struct _state_transition_t {
    const state initial_state;
    const state signal_state;
    const state final_state;
};

// supported state transitions
const state_transition_t INITIALIZE = {UNINITIALIZED, INITIALIZING, IDLE};
const state_transition_t CONFIGURE = {IDLE, CONFIGURING, CONFIGURED};
const state_transition_t START_RUN = {CONFIGURED, STARTING, RUNNING};
const state_transition_t STOP_RUN = {RUNNING, STOPPING, CONFIGURED};

state_ctrl_t *
state_ctrl_new(state_component_t component)
{
    state_ctrl_t *self = (state_ctrl_t *)calloc(1, sizeof(state_ctrl_t));
    assert(self);

    self->state = UNINITIALIZED;

    assert(pthread_mutex_init(&(self->lock), NULL) == 0);

    self->component = component;

    return self;
}

void
state_ctrl_destroy(state_ctrl_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        state_ctrl_t *self = *self_p;

        free(self);
        *self_p = NULL;
    }
}

//  launches the controller thread
bool
state_ctrl_start(state_ctrl_t *self)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (pthread_create(&(self->ctrl_th), NULL, &_state_ctrl_main, self) != 0) {
        return false;
    }

    pthread_attr_destroy(&attr);

    return true;
}

// block waiting for controller thread to exit
bool
state_ctrl_join(state_ctrl_t *self)
{
    void *exit_data;
    int status = pthread_join(self->ctrl_th, &exit_data);

    if (status) {
        printf("Error waiting for ctrl_th thread %d\n", status);
    }

    return true;
}

state
state_ctrl_get_state(state_ctrl_t *self)
{
    _lock(self);
    state tmp = self->state;
    _unlock(self);
    return tmp;
}

// perform a state transition on a group of controllers en masse.
void
state_ctrl_transition_state(state_ctrl_t *ctrl_v[], size_t len, const state_transition_t *transition)
{
    // todo limit the max spin wait, returning fault if transition can not be  completed

    // assert precondition
    for (int i = 0; i < len; i++) {
        assert(state_ctrl_get_state(ctrl_v[i]) == transition->initial_state);
    }

    // signal controllers
    for (int i = 0; i < len; i++) {
        _state_ctrl_set_state(ctrl_v[i], transition->signal_state);
    }

    // spin wait for controllers to get to desired state
    bool ready = true;
    int loops = 0;
    struct timespec poll;
    poll.tv_sec = 0;
    poll.tv_nsec = 10000000; // 10 ms poll for idle loops
    do {
        ready = true;
        for (int i = 0; i < len; i++) {
            if (ctrl_v[i]) {
                ready = ready & (state_ctrl_get_state(ctrl_v[i]) == transition->final_state);
            }
        }
        if (!ready) {
            nanosleep(&poll, NULL);
            // sleep(1);
        }

        if ((++loops % 1000) == 0) {
            printf("WARN: spin waiting for controllers to move to state %d\n", transition->final_state);
        }

    } while (!ready);
}

static inline void
_lock(state_ctrl_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static inline void
_unlock(state_ctrl_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static void
_state_ctrl_set_state(state_ctrl_t *self, state state)
{
    _lock(self);
    self->state = state;
    _unlock(self);
}

// main loop
static void *
_state_ctrl_main(void *arg)
{

    state_ctrl_t *self = (state_ctrl_t *)arg;

    struct timespec rest;
    rest.tv_sec = 0;
    // rest.tv_nsec = 100000000; // 100 ms rest for idle loops
    rest.tv_nsec = 10000000; // 10 ms rest for idle loops

    while (true) {
        bool pause = false;
        switch (state_ctrl_get_state(self)) {
        case INITIALIZING: {
            bool success = (*(self->component.init_action))(self->component.ctx);
            _state_ctrl_set_state(self, success ? IDLE : ERROR);
            break;
        }
        case IDLE: {
            pause = true;
            break;
        }
        case CONFIGURING: {
            bool success = (*(self->component.config_action))(self->component.ctx);
            _state_ctrl_set_state(self, success ? CONFIGURED : ERROR);
            break;
        }
        case CONFIGURED: {
            pause = true;
            break;
        }
        case STARTING: {
            bool success = (*(self->component.start_run_action))(self->component.ctx);
            _state_ctrl_set_state(self, success ? RUNNING : ERROR);
            break;
        }
        case RUNNING: {
            // todo learn pause from component
            bool success = (*(self->component.running_action))(self->component.ctx, &pause);
            if (!success) {
                _state_ctrl_set_state(self, ERROR);
            }
            break;
        }
        case STOPPING: {
            bool success = (*(self->component.stop_run_action))(self->component.ctx);
            _state_ctrl_set_state(self, success ? CONFIGURED : ERROR);
            break;
        }
        case ERROR: {
            pause = true;
            break;
        }
        default: {
            break;
        }
        }
        if (pause) {
            nanosleep(&rest, NULL);
        }
        // if(pause){sleep(1);}
    }
}
