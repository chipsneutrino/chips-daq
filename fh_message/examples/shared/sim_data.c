/**
 * sim_data.c
 *
 * Simulate a microdaq source with an asynch producer pushing simulated hits
 * into a hit buffer.
 * 
 */
 #include "standard_inc.h"
#include <pthread.h>

#include "sim_data.h"


// forwards
static inline void _lock(sim_data_t *self);
static inline void _unlock(sim_data_t *self);
static void* _producer_loop(void *args);
void _produce(sim_data_t *self, double current_nano);
void _generate_record_type_0(sim_data_t *self, time_t cur_year, time_t cur_second, uint32_t timestamp);
void _generate_record_type_1(sim_data_t *self, time_t cur_year, time_t cur_second, uint32_t timestamp);
void _bootstrap_current_year(sim_data_t *self);

// accounting info driving hit production
typedef struct {
    uint32_t cur_year;       // the current year
    uint32_t cur_year_start;  // the number of seconds (from the unix epoc) at start of year ... in local timezone
    uint32_t hit_interval;    // interval between hits (microseconds)
    double start_nano;        // monotonic starting point of data prodcution
    uint32_t last_second;     // second value of last hit emitted
    uint32_t last_offset;     // offset value of last hit emitted (microseconds)
    double last_nano;         // monotonic point-in-time of last hit emitted

} xyzzy;

// instance data
struct _sim_data_t {
    sim_spec spec;           // production config
    hitbuf_t *hit_buffer;    // hit buffer
    pthread_t producer_th;   // producer thread
    pthread_mutex_t lock;    // pthread mutex
    int period;              // controls production frequency
    xyzzy prod_status;       // hit production status
};



//  Create a new sim data
sim_data_t *
sim_data_new(hitbuf_t *hit_buffer, sim_spec spec)
{
    sim_data_t *self = (sim_data_t *)calloc(1,sizeof(sim_data_t));
    assert(self);

    self->hit_buffer = hit_buffer;
    self->spec.rps = spec.rps;
    self->period = 500;
    
    assert(pthread_mutex_init(&(self->lock), NULL) == 0);
    return self;
}

//  Destroy the sim data
void
sim_data_destroy(sim_data_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        sim_data_t *self = *self_p;
        pthread_mutex_destroy(&(self->lock));
        free(self);
        *self_p = NULL;
    }
}


//  launches the data prodcution thread
bool
sim_data_start(sim_data_t *self)
{
    // activate publisher loop
    if (pthread_create(&(self->producer_th), NULL, &_producer_loop, self) != 0) {
        return false;
    }

    return true;

}

static inline void
_lock(sim_data_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static inline  void
_unlock(sim_data_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static void *
_producer_loop(void *args)
{

    sim_data_t *self = (sim_data_t *)args;

    _bootstrap_current_year(self);

    self->prod_status.hit_interval = 1e6 / self->spec.rps; //in microseconds

    // Note: a rough approximation to match the second boundary to a monotonic timestamp
    // using two separate calls to red the clock.

    // get the monotonic time
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    self->prod_status.last_nano = ts.tv_sec * 1e9 + ts.tv_nsec;

    // get the wall clock time
    clock_gettime(CLOCK_REALTIME, &ts);
    self->prod_status.last_second = ts.tv_sec - self->prod_status.cur_year_start;
    self->prod_status.last_offset = ts.tv_nsec;

    while (true) {
        usleep(self->period * 1000);

        _lock(self);

        clock_gettime(CLOCK_MONOTONIC, &ts);
        double current_nano = ts.tv_sec * 1e9 + ts.tv_nsec;
        _produce(self, current_nano);

        _unlock(self);
    }

    return NULL;
}

void
_produce(sim_data_t *self, double current_nano)
{
    // debug
    //double elapsed = current_nano - self->prod_status.last_nano;
    //int count = (self->spec.rps * elapsed / 1e9);
    //printf("elapsed millis: %f, hits: %u\n", elapsed / 1e6, count);

    //int cnt = 0;
    while ((self->prod_status.last_nano + (self->prod_status.hit_interval * 1e3) <= current_nano)) {
        self->prod_status.last_offset += self->prod_status.hit_interval;
        if (self->prod_status.last_offset >= 1e6) {
            self->prod_status.last_second += 1;
            self->prod_status.last_offset -= 1e6;
        }
        self->prod_status.last_nano += (self->prod_status.hit_interval * 1e3);
        // emit hit w/second + offset
        _generate_record_type_1(self, self->prod_status.cur_year, self->prod_status.last_second, self->prod_status.last_offset);

        //printf("-->hit at sec: %u,   offset: %u\n", self->prod_status.last_second, self->prod_status.last_offset);
        //cnt++;
    }
    //printf("produced %d hits ... last_nano: %.0f, current_nano: %.0f, hit_interval: %u\n", cnt,
     //self->prod_status.last_nano, current_nano, self->prod_status.hit_interval);
}

void
_generate_record_type_0(sim_data_t *self, time_t cur_year, time_t cur_second, uint32_t timestamp)
{
    hit_buf_add_trig_t0(self->hit_buffer, cur_year, cur_second, timestamp);
}

void
_generate_record_type_1(sim_data_t *self, time_t cur_year, time_t cur_second, uint32_t timestamp)
{
    //printf("sec: %ld, off: %u\n", cur_second, timestamp);
    int tot = rand() % 0x3FF;
    int adc_cnt = rand() % 16;

    uint16_t adc[16];
    for (int i = 0; i < adc_cnt; i++) {
        adc[i] = (i << 12); // ADC ID
        adc[i] |= (rand() % 0x3FF); //ADC value
    }
    hit_buf_add_trig_t1(self->hit_buffer, cur_year, cur_second, timestamp, tot, adc, adc_cnt);
}

// populate the current year and the unix time at the start of the year
void
_bootstrap_current_year(sim_data_t *self)
{
    time_t now;
    time(&now);
    struct tm *local_time = localtime(&now);
    self->prod_status.cur_year = local_time->tm_year + 1900;

    // unix time value at start of the year
    local_time->tm_mon = 0;
    local_time->tm_mday = 1;
    local_time->tm_hour = 0;
    local_time->tm_min = 0;
    local_time->tm_sec = 0;
    self->prod_status.cur_year_start = mktime(local_time); //NOTE: in the local timezone

}
