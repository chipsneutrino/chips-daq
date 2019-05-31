/**
 * sim_bbb.c
 *
 * Simulate a beagle bone field hub.
 */
#include "standard_inc.h"
#include <pthread.h>

#include "bbb_comms_api.h"
#include "data_access_service.h"
#include "fh_connector.h"
#include "msg_service.h"
#include "pagebuf.h"
#include "sim_bbb.h"
#include "udaq_comms_api.h"
#include "udaq_comp.h"
#include "udaq_comp_group.h"
#include "udaq_peer.h"

#define NUM_UARTS 4
#define NUM_UDAQ_PER_UART 4

// forwards
static inline void _lock(sim_bbb_t *self);
static inline void _unlock(sim_bbb_t *self);
static void * _main_loop(void *args);
static void * _sdaq_loop(void *args);
static void _sim_bb_store_page(void *ctx, uint8_t channel_id, uint8_t *page, size_t len);
static bool _sim_bb_adapt_hit_buffer(void *ctx, page_sink *sink);
static int _handle_configure(void *ctx, fh_message_t *msgin, fh_transport_t *transport);
static int _handle_start_run(void *ctx, fh_message_t *msgin, fh_transport_t *transport);
static int _handle_stop_run(void *ctx, fh_message_t *msgin, fh_transport_t *transport);

// holds objects related to the field hub services
typedef struct {
    fh_connector_t *connector;     // used to establish connection
    fh_dispatch_t *dispatch;       // binds incoming messages to handler functions
    fh_transport_t *transport;     // connection to client (sdaq)
    fh_message_t *msg;             // message structure used for comms
    ms_service_t *msg_service;     // handlers for message service
    ds_service_t *data_service;    // handlers for data service
    fh_service_t *fh_ctrl_service; // handlers for field hub control service
    bool trace_protocol;           // debugging mode for msg layer
    bool trace_stream;             // debugging mode for stream layer
    FILE *trace_out;               // debugging destination for msg trace
} fh_services_t;

// instance data
struct _sim_bbb_t {
    pagebuf_t *pagebuf;          // central store of aquired udaq pages
    pthread_t main_th;           // main thread
    pthread_t sdaq_th;           // thread servicing sdaqs
    pthread_mutex_t lock;        // pthread mutex
    bool running;                // runnning status
    udaq_comp_group_t **group_v; // groups of udaqs components sharing the same state controller thread.
                                 //   (The udaq thread model is organized by UART such that udaqs
                                 //     on the same UART will have communications serialize)
    state_ctrl_t **ctrl_v;       // controllers managing udaq_comp_groups
    size_t num_groups;           // number of component groups
    pthread_cond_t connect_cv;   // condition variable for new client connection
    fh_services_t fh_services;   // holds hub services
};

//  Create a new sim bbb
sim_bbb_t *
sim_bbb_new(fh_connector_t *sdaq_connector, uint8_t buffer_sz, uint8_t page_sz)
{
    sim_bbb_t *self = (sim_bbb_t *)calloc(1, sizeof(sim_bbb_t));
    assert(self);

    self->pagebuf = pagebuf_new(buffer_sz, page_sz); // should match udaq pages

    self->num_groups = NUM_UARTS; // todo pass in as argument
    self->group_v = (udaq_comp_group_t **)calloc(1, sizeof(udaq_comp_group_t *) * self->num_groups);
    assert(self->group_v);
    self->ctrl_v = (state_ctrl_t **)calloc(1, sizeof(state_ctrl_t *) * self->num_groups);
    assert(self->ctrl_v);

    for (int i = 0; i < self->num_groups; i++) {
        self->group_v[i] = udaq_comp_group_new(NUM_UDAQ_PER_UART);

        state_component_t state_comp;
        udaq_comp_group_as_state_comp(self->group_v[i], &state_comp);
        self->ctrl_v[i] = state_ctrl_new(state_comp);
    }

    assert(pthread_mutex_init(&(self->lock), NULL) == 0);
    assert(pthread_cond_init(&(self->connect_cv), NULL) == 0);
    // assert(pthread_cond_init (&(self->connect_cv), NULL) == 0);

    // set up the sdaq interface
    self->fh_services.connector = sdaq_connector;
    self->fh_services.dispatch = fh_dispatch_new();
    self->fh_services.transport = NULL;
    self->fh_services.msg = fh_message_new();

    // message service
    ms_service_t *msg_service = ms_new(MSG_SERVICE, NULL);
    ms_register_service(msg_service, self->fh_services.dispatch);
    self->fh_services.msg_service = msg_service;

    // data access service
    ds_service_t *data_service =
        ds_new(DATA_ACCESS_SERVICE, &(page_buffer){.ctx = self, .pop_page = &_sim_bb_adapt_hit_buffer});
    ds_register_service(data_service, self->fh_services.dispatch);
    self->fh_services.data_service = data_service;

    // field hub control service (in-file impl)
    fh_service_t *fh_ctrl_service = fh_service_new(FH_CTRL_SERVICE, self);
    fh_service_register_function(fh_ctrl_service, FH_CONFIGURE, &_handle_configure);
    fh_service_register_function(fh_ctrl_service, FH_START_RUN, &_handle_start_run);
    fh_service_register_function(fh_ctrl_service, FH_STOP_RUN, &_handle_stop_run);
    fh_dispatch_register_service(self->fh_services.dispatch, fh_ctrl_service);
    self->fh_services.fh_ctrl_service = fh_ctrl_service;

    return self;
}

//  Destroy the sim bbb
void
sim_bbb_destroy(sim_bbb_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        sim_bbb_t *self = *self_p;
        pthread_mutex_destroy(&(self->lock));
        // pthread_cond_destroy(&(self->connect_cv));

        for (int i = 0; i < self->num_groups; i++) {
            udaq_comp_group_destroy(&(self->group_v[i]));
            state_ctrl_destroy(&(self->ctrl_v[i]));
        }

        ms_destroy(&(self->fh_services.msg_service));
        fh_service_destroy(&(self->fh_services.fh_ctrl_service));
        ds_destroy(&(self->fh_services.data_service));

        fh_connector_destroy(&(self->fh_services.connector));
        if (self->fh_services.transport) {
            fh_transport_destroy(&(self->fh_services.transport));
        }
        fh_dispatch_destroy(&(self->fh_services.dispatch));
        fh_message_destroy(&(self->fh_services.msg));

        free(self);
        *self_p = NULL;
    }
}

//  launches service threads
bool
sim_bbb_start(sim_bbb_t *self)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (pthread_create(&(self->main_th), NULL, &_main_loop, self) != 0) {
        return false;
    }

    if (pthread_create(&(self->sdaq_th), NULL, &_sdaq_loop, self) != 0) {
        return false;
    }

    pthread_attr_destroy(&attr);

    return true;
}

// block waiting for bbb main loop to exit.
void
sim_bbb_join(sim_bbb_t *self)
{
    void *exit_data;
    int status = pthread_join(self->main_th, &exit_data);

    if (status) {
        printf("Error waiting for main thread %d\n", status);
    }
}

// configure a udaq
void
sim_bbb_configure_udaq(sim_bbb_t *self, udaq_peer_t *udaq, uint8_t thread_group)
{
    assert(thread_group < self->num_groups);
    _lock(self);

    udaq_comp_group_add_udaq(self->group_v[thread_group],
                             udaq_comp_new(udaq, (page_handler){.ctx = self, .handle_page = &_sim_bb_store_page}));
    _unlock(self);
}

void
sim_bbb_configure(sim_bbb_t *self)
{

    // NOTE: The what/where/how details of udaq connections should
    //      be passed in as part of the configuration. For now we
    //      call a global shim method
    configure_udaqs(self);

    // start controllers
    for (int i = 0; i < self->num_groups; i++) {
        state_ctrl_start(self->ctrl_v[i]);
    }

    // initialize udaq controllers
    state_ctrl_transition_state(self->ctrl_v, self->num_groups, &INITIALIZE);

    // configure udaq controllers
    state_ctrl_transition_state(self->ctrl_v, self->num_groups, &CONFIGURE);
}

void
sim_bbb_start_run(sim_bbb_t *self)
{
    // start run on udaq controllers
    state_ctrl_transition_state(self->ctrl_v, NUM_UARTS, &START_RUN);
}

void
sim_bbb_stop_run(sim_bbb_t *self)
{
    // stop run on udaq controllers
    state_ctrl_transition_state(self->ctrl_v, NUM_UARTS, &STOP_RUN);
}

// enable tracing of messages
void
sim_bbb_trace_protocol(sim_bbb_t *self, FILE *out)
{
    self->fh_services.trace_protocol = true;
    self->fh_services.trace_out = out;
    if (self->fh_services.transport) {
        fh_transport_enable_protocol_trace(self->fh_services.transport, self->fh_services.trace_out);
    }
}

// enable tracing of stream operations
void
sim_bbb_trace_stream(sim_bbb_t *self, FILE *out)
{
    self->fh_services.trace_stream = true;
    self->fh_services.trace_out = out;
    if (self->fh_services.transport) {
        fh_transport_enable_stream_trace(self->fh_services.transport, self->fh_services.trace_out);
    }
}

static void
_sim_bb_store_page(void *ctx, uint8_t channel_id, uint8_t *page, size_t len)
{
    assert(len >= 4);
    sim_bbb_t *self = (sim_bbb_t *)ctx;

    // store the channel id into the reseved space in the low 10 bits of the first word
    *page = *page | channel_id;

    bool overflowed = pagebuf_push_page(self->pagebuf, page, len);
    if (overflowed) {
        // todo this is difficult to signal to sdaq without altering
        //     the page structure.
        printf("XXX Page Buffer Overflow\n");
    }
}

static bool
_sim_bb_adapt_hit_buffer(void *ctx, page_sink *sink)
{
    sim_bbb_t *self = (sim_bbb_t *)ctx;
    return pagebuf_pop_page_to(self->pagebuf, sink);
}

static inline void
_lock(sim_bbb_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static inline void
_unlock(sim_bbb_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

// main thread
static void *
_main_loop(void *args)
{

    sim_bbb_t *self = (sim_bbb_t *)args;
    self->running = true;

    while (self->running) {
        sleep(10);

        pagebuf_stats stats;
        pagebuf_get_stats(self->pagebuf, &stats);

        printf("xxx: pages_pushed[%d], pages_popped[%d], pages_overflowed[%d], pages_held[%d]\n", stats.pushed,
               stats.popped, stats.overflows, (stats.pushed - stats.popped - stats.overflows));
    }

    // wait for controllers
    for (int i = 0; i < self->num_groups; i++) {
        state_ctrl_join(self->ctrl_v[i]);
    }

    // to do join on other threads

    return NULL;
}

// thread serving the sdaq interface
static void *
_sdaq_loop(void *args)
{
    sim_bbb_t *self = (sim_bbb_t *)args;

    // block waiting for a client connection
    fh_stream_t *stream = fh_connector_connect(self->fh_services.connector);
    if (!stream) {
        // todo handle connection fault
        printf("client connection ERROR.\n");
        return NULL;
    }
    else {
        self->fh_services.transport = fh_transport_new(fh_protocol_new_plain(), stream);
        printf("client connect OK.\n");
    }

    // enable tracing?
    if (self->fh_services.trace_protocol) {
        fh_transport_enable_protocol_trace(self->fh_services.transport, self->fh_services.trace_out);
    }
    if (self->fh_services.trace_stream) {
        fh_transport_enable_stream_trace(self->fh_services.transport, self->fh_services.trace_out);
    }

    // local aliases
    fh_message_t *msg = self->fh_services.msg;
    fh_dispatch_t *dispatch = self->fh_services.dispatch;
    fh_transport_t *transport = self->fh_services.transport;

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

// ############################################################################
// field hub control service
// ############################################################################

static int
_handle_configure(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    sim_bbb_t *self = (sim_bbb_t *)ctx;

    printf("XXX GOT CONFIG\n");

    sim_bbb_configure(self);

    uint8_t out[] = {1}; // OK
    fh_message_setData(msgin, out, 1);
    return fh_transport_send(transport, msgin);
}

static int
_handle_start_run(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    sim_bbb_t *self = (sim_bbb_t *)ctx;

    printf("XXX GOT START RUN\n");

    sim_bbb_start_run(self);

    uint8_t out[] = {1}; // OK
    fh_message_setData(msgin, out, 1);
    return fh_transport_send(transport, msgin);
}

static int
_handle_stop_run(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    sim_bbb_t *self = (sim_bbb_t *)ctx;

    sim_bbb_stop_run(self);

    uint8_t out[] = {1}; // OK
    fh_message_setData(msgin, out, 1);
    return fh_transport_send(transport, msgin);
}
