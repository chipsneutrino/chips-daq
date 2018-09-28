/**
 * sim_sdaq.c
 *
 * Simulate an sdaq.
 */

#include "standard_inc.h"
#include <pthread.h>

#include "fh_comp.h"
#include "fh_connector.h"
#include "fh_peer.h"
#include "sim_sdaq.h"

// forwards
static inline void _lock(sim_sdaq_t *self);
static inline void _unlock(sim_sdaq_t *self);
static void * _main_loop(void *args);
static void _sim_sdaq_store_page(void *ctx, uint8_t bbb_id, uint8_t *page, size_t len);

#define MAX_FIELD_HUBS 10

// throughput stats
typedef struct {
    uint32_t id;
    uint64_t start_nanos;
    uint64_t last_nanos;
    uint32_t page_in;
    uint32_t bytes_in;
    uint32_t good_bytes_in;
} pg_throughput_t;

// instance data
struct _sim_sdaq_t {
    pthread_t main_th;                      // main thread
    pthread_mutex_t lock;                   // pthread mutex
    bool running;                           // running status
    fh_comp_t *comp_v[MAX_FIELD_HUBS];      // storage for field hub components
    state_ctrl_t *ctrl_v[MAX_FIELD_HUBS];   // storage for component state controllers
    pg_throughput_t stat_v[MAX_FIELD_HUBS]; // storage for throughput stats
    size_t num_hubs;                        // number of hub components
};

// Create a new sim sdaq
sim_sdaq_t *
sim_sdaq_new()
{
    sim_sdaq_t *self = (sim_sdaq_t *)calloc(1, sizeof(sim_sdaq_t));
    assert(self);

    assert(pthread_mutex_init(&(self->lock), NULL) == 0);

    return self;
}

// Destroy the sim sdaq
void
sim_sdaq_destroy(sim_sdaq_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        sim_sdaq_t *self = *self_p;

        // populated during config
        for (int i = 0; i < self->num_hubs; i++) {

            if (self->comp_v[i]) {
                fh_comp_destroy(&(self->comp_v[i])); // destroy controller
            }

            if (self->ctrl_v[i]) {
                state_ctrl_destroy(&(self->ctrl_v[i])); // destroy controller
            }
        }

        pthread_mutex_destroy(&(self->lock));

        free(self);
        *self_p = NULL;
    }
}

// launches component threads
bool
sim_sdaq_start(sim_sdaq_t *self)
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

// block waiting for main loop to exit.
void
sim_sdaq_join(sim_sdaq_t *self)
{
    void *exit_data;
    int status = pthread_join(self->main_th, &exit_data);

    if (status) {
        printf("Error witing for main thread %d\n", status);
    }
}

// add a field hub peer
void
sim_sdaq_configure_fh(sim_sdaq_t *self, fh_peer_t *bbb)
{
    // failed for no free slots
    assert(self->num_hubs < MAX_FIELD_HUBS);

    _lock(self);
    self->stat_v[self->num_hubs].id = self->num_hubs;
    self->comp_v[self->num_hubs] =
        fh_comp_new(bbb, (page_handler){.ctx = &(self->stat_v[self->num_hubs]), .handle_page = &_sim_sdaq_store_page});
    state_component_t state_comp;

    fh_comp_as_state_comp(self->comp_v[self->num_hubs], &state_comp);

    self->ctrl_v[self->num_hubs] = state_ctrl_new(state_comp);

    self->num_hubs++;
    _unlock(self);
}

void
sim_sdaq_configure(sim_sdaq_t *self)
{
    // NOTE: The what/where/how details of udaq connections should
    //      be passed in as part of the configuration. For now we
    //      call a global shim method
    configure_field_hubs(self);

    // launch controller threads
    for (int i = 0; i < self->num_hubs; i++) {
        state_ctrl_start(self->ctrl_v[i]);
    }

    // initialize field hub components
    state_ctrl_transition_state(self->ctrl_v, self->num_hubs, &INITIALIZE);

    // configure field hub components
    state_ctrl_transition_state(self->ctrl_v, self->num_hubs, &CONFIGURE);
}

void
sim_sdaq_start_run(sim_sdaq_t *self)
{
    // start run on field hub components
    state_ctrl_transition_state(self->ctrl_v, self->num_hubs, &START_RUN);
}

void
sim_sdaq_stop_run(sim_sdaq_t *self)
{
    // stop run on field hub components
    state_ctrl_transition_state(self->ctrl_v, self->num_hubs, &STOP_RUN);
}

static inline void
_lock(sim_sdaq_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static inline void
_unlock(sim_sdaq_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static void *
_main_loop(void *args)
{

    sim_sdaq_t *self = (sim_sdaq_t *)args;
    self->running = true;

    while (self->running) {
        sleep(10);
        // todo print page transfer state
        for (int i = 0; i < self->num_hubs; i++) {
            pg_throughput_t *stats = &self->stat_v[i];
            uint32_t mb = (stats->bytes_in / 1024.0);
            double mbps = 0;
            if (stats->start_nanos != 0) {
                mbps = mb / ((stats->last_nanos - stats->start_nanos) * 1e-9);
            }
            printf("field hub: %d, mb: %d, mbps: %f\n", i, mb, mbps);
        }
    }

    return NULL;
}

static uint64_t
now()
{
    // get the monotonic time
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t nanos = ((uint64_t)ts.tv_sec) * 1e9 + ts.tv_nsec;
    return nanos;
}

static void
_sim_sdaq_store_page(void *ctx, uint8_t bbb_id, uint8_t *page, size_t len)
{
    // todo spool
    pg_throughput_t *stats = (pg_throughput_t *)ctx;

    uint64_t t = now();

    uint32_t page_header = *((uint32_t *)page);
    // uint8_t udaq_channel = page_header & 0x3FF;
    uint16_t utilized = (page_header & 0x03FFFC00) >> 10;

    uint32_t mb = (stats->bytes_in / 1024.0);
    double mbps = 0;
    if (stats->start_nanos == 0) {
        stats->start_nanos = t;
    }
    else {
        mbps = mb / ((t - stats->start_nanos) * 1e-9);
    }
    stats->page_in++;
    stats->bytes_in += len;
    stats->good_bytes_in += utilized;
    stats->last_nanos = t;

    // printf("got page[%zu] utilized[%d] from field hub %d, udaq channel %d, mb: %d, mbps: %f\n", len, utilized,
    // bbb_id, udaq_channel, mb, mbps );
}
