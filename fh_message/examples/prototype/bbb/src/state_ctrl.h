/**
 * state_ctrl.h
 *
 * Manages state transitions of a component.
 */
#include <stdbool.h>


#ifndef STATE_CTRL_INCLUDED
#define STATE_CTRL_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _state_ctrl_t state_ctrl_t;

// interface to the component under control
typedef bool (*action_fn)(void *ctx);
typedef bool (*running_fn)(void *ctx, bool *yield);
typedef struct {
  void * ctx;
  action_fn init_action;
  action_fn config_action;
  action_fn start_run_action;
  running_fn running_action;
  action_fn stop_run_action;
} state_component_t;

typedef enum {
  UNINITIALIZED,
  INITIALIZING,
  IDLE,
  CONFIGURING,
  CONFIGURED,
  STARTING,
  RUNNING,
  STOPPING,
  ERROR
} state;

// models an asyncronous state transition
typedef struct _state_transition_t state_transition_t;

// supported state transitions
extern const state_transition_t INITIALIZE;
extern const state_transition_t CONFIGURE;
extern const state_transition_t START_RUN;
extern const state_transition_t STOP_RUN;


state_ctrl_t*  
state_ctrl_new(state_component_t component);

void
state_ctrl_destroy(state_ctrl_t **self_p);

// launches the controller thread
bool
state_ctrl_start(state_ctrl_t *self);

// block waiting for controller thread to exit
bool
state_ctrl_join(state_ctrl_t *self);


// get the current state
state
state_ctrl_get_state(state_ctrl_t *self);

// perform a state transition on a group of controllers en masse.
// This allows for concurrency across controllers.
void
state_ctrl_transition_state(state_ctrl_t *ctrl_v[], size_t len, const state_transition_t *transition);

#ifdef __cplusplus
}
#endif

#endif
