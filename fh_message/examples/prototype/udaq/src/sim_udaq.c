#include "standard_inc.h"
#include <pthread.h>

#include "sim_udaq.h"
#include "udaq_comms_api.h"

#include "msg_service.h"
#include "data_access_service.h"

#include "hitbuf.h"
#include "sim_data.h"


// ------------------------------------------------------------------
// Roughly simulates a microdaq.
//
// supported functions:
//
// MESSAGE_SERVICE: 1
//  [1,1] status
//  [1,2] echo
//  [1,3] close
//
// DATA_ACCESS_SERVICE: 3
//  [3,1] poll page
//
// EXP_CTRL_SERVICE: 4
//  [4,1] start run
//  [4,2] stop run
//
//
// ------------------------------------------------------------------


// forwards
static inline void _lock(sim_udaq_t *self);
static inline void _unlock(sim_udaq_t *self);
static void* _main_loop(void *args);
bool _adapt_hit_buffer(void* ctx, page_sink *sink);

// exp_ctrl_service handlers
static int _handle_start_run(void *ctx, fh_message_t *msgin, fh_transport_t *transport);
static int _handle_stop_run(void *ctx, fh_message_t *msgin, fh_transport_t *transport);

// holds objects related to the udaq services stack
typedef struct {
    fh_connector_t *connector;      // used to establish connection
    fh_dispatch_t *dispatch;        // binds incoming messages to handler functions
    fh_transport_t *transport;      // connection to client (field hub)
    fh_message_t *msg;              // message structure used for comms
    ms_service_t *msg_service;      // handlers for message service
    fh_service_t *exp_ctrl_service; // handlers for experiment control service
    ds_service_t *data_service;     // handlers for data service
    bool trace_protocol;            // debugging mode for msg layer
    bool trace_stream;              // debugging mode for stream layer
    FILE *trace_out;                // debugging destination for msg trace
} udaq_services_t;

// instance data
struct _sim_udaq_t {
    sim_udaq_spec spec;            // config settings
    pthread_t main_th;             // main thread
    pthread_mutex_t lock;          // pthread mutex
    pthread_cond_t connect_cv;     // condition variable for connection
    hitbuf_t *hitbuf;              // hit buffer
    sim_data_t *sim_data;          // simulated data producer
    udaq_services_t udaq_services; // holds udaq services
    bool running;
};

//  Create a new sim udaq
sim_udaq_t *
sim_udaq_new(sim_udaq_spec spec, fh_connector_t *hub_connector)
{
    sim_udaq_t *self = (sim_udaq_t *)calloc(1, sizeof(sim_udaq_t));
    assert(self);

    self->spec.buffer_sz = spec.buffer_sz;
    self->spec.page_sz = spec.page_sz;
    self->spec.hit_rate = spec.hit_rate;

    self->hitbuf = hitbuf_new(self->spec.buffer_sz, self->spec.page_sz);

    self->sim_data = sim_data_new(self->hitbuf, (sim_spec){self->spec.hit_rate});

    // set up the udaq interface
    self->udaq_services.connector = hub_connector;
    self->udaq_services.dispatch = fh_dispatch_new();
    self->udaq_services.transport = NULL;
    self->udaq_services.msg = fh_message_new();

    // message service
    ms_service_t *msg_service = ms_new(MSG_SERVICE, NULL);
    ms_register_service(msg_service, self->udaq_services.dispatch);
    self->udaq_services.msg_service = msg_service;

    // data service
    ds_service_t *data_service =
        ds_new(DATA_ACCESS_SERVICE, &(page_buffer){.ctx = self->hitbuf, .pop_page = &_adapt_hit_buffer});
    ds_register_service(data_service, self->udaq_services.dispatch);
    self->udaq_services.data_service = data_service;

    // experiment serice (in-file impl)
    fh_service_t *exp_ctrl_service = fh_service_new(EXP_CTRL_SERVICE, self);
    fh_service_register_function(exp_ctrl_service, ECS_START_RUN, &_handle_start_run);
    fh_service_register_function(exp_ctrl_service, ECS_STOP_RUN, &_handle_stop_run);
    fh_dispatch_register_service(self->udaq_services.dispatch, exp_ctrl_service);
    self->udaq_services.exp_ctrl_service = exp_ctrl_service;

    assert(pthread_mutex_init(&(self->lock), NULL) == 0);
    assert(pthread_cond_init(&(self->connect_cv), NULL) == 0);
    return self;
}

//  Destroy the sim udaq
void
sim_udaq_destroy(sim_udaq_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        sim_udaq_t *self = *self_p;
        pthread_mutex_destroy(&(self->lock));
        pthread_cond_destroy(&(self->connect_cv));

        ms_destroy(&(self->udaq_services.msg_service));
        ds_destroy(&(self->udaq_services.data_service));
        fh_service_destroy(&(self->udaq_services.exp_ctrl_service));

        fh_connector_destroy(&(self->udaq_services.connector));
        fh_dispatch_destroy(&(self->udaq_services.dispatch));
        if (self->udaq_services.transport) {
            fh_transport_destroy(&(self->udaq_services.transport));
        }
        fh_message_destroy(&(self->udaq_services.msg));

        free(self);
        *self_p = NULL;
    }
}

//  launches the main thread
bool
sim_udaq_start(sim_udaq_t *self)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (pthread_create(&(self->main_th), NULL, &_main_loop, self) != 0) {
        return false;
    }

    pthread_attr_destroy(&attr);

    return true;
}

// connect a client control channel
void
sim_udaq_connect(sim_udaq_t *self, fh_transport_t *transport)
{
    _lock(self);
    self->udaq_services.transport = transport;
    pthread_cond_signal(&(self->connect_cv));
    _unlock(self);
}

// block waiting for udaq main loop to exit.
void
sim_udaq_join(sim_udaq_t *self)
{
    void *exit_status;
    pthread_join(self->main_th, &exit_status);
}

static inline void
_lock(sim_udaq_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static inline void
_unlock(sim_udaq_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static void *
_main_loop(void *args)
{

    sim_udaq_t *self = (sim_udaq_t *)args;
    self->running = true;

    // block waiting for a client connection
    fh_stream_t *stream = fh_connector_connect(self->udaq_services.connector);
    if (!stream) {
        // todo handle connection fault
        printf("client connection ERROR.\n");
        return NULL;
    }
    else {
        self->udaq_services.transport = fh_transport_new(fh_protocol_new_plain(), stream);
        printf("client connect OK.\n");
    }

    // enable tracing?
    if (self->udaq_services.trace_protocol) {
        fh_transport_enable_protocol_trace(self->udaq_services.transport, self->udaq_services.trace_out);
    }
    if (self->udaq_services.trace_stream) {
        fh_transport_enable_stream_trace(self->udaq_services.transport, self->udaq_services.trace_out);
    }

    // local aliases
    fh_message_t *msg = self->udaq_services.msg;
    fh_dispatch_t *dispatch = self->udaq_services.dispatch;
    fh_transport_t *transport = self->udaq_services.transport;

    int status = 0;
    while (self->running) {
        if ((status = fh_transport_receive(transport, msg)) == 0) {
            if ((status = fh_dispatch_handle(dispatch, msg, transport)) != 0) {
                printf("Transport error: %d\n", status);
                break;
            }
        }
        else {
            printf("Transport error: %d\n", status);
            break;
        }
    }

    return NULL;
}

bool
_adapt_hit_buffer(void *ctx, page_sink *sink)
{
    return hitbuf_pop_page_to((hitbuf_t *)ctx, sink);
}

// ############################################################################
// experiment control service
// ############################################################################
static int
_handle_start_run(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    sim_udaq_t *self = (sim_udaq_t *)ctx;

    // fire up the data similator
    sim_data_start(self->sim_data);

    uint8_t out[] = {1}; // OK
    fh_message_setData(msgin, out, 1);
    return fh_transport_send(transport, msgin);
}

static int
_handle_stop_run(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    // sim_udaq_t *self = (sim_udaq_t*)ctx;

    // todo: stop the data similator
    // sim_data_stop(self->sim_data);

    uint8_t out[] = {1}; // OK
    fh_message_setData(msgin, out, 1);
    return fh_transport_send(transport, msgin);
}
