/**
 * fh_error_stream.c
 *
 * Implements a stream decorator that injects bit errors into the stream.
 * Used for testing protocol implementations.
 */
#include "../fh_classes.h"

typedef struct _fh_error_stream_t fh_error_stream_t;
typedef struct _fh_mangle_t fh_mangle_t;


typedef struct {
uint32_t min;
uint32_t max;
} int_range_t;

typedef struct {
float min;
float max;
} float_range_t;


// forwards
static void logtrace(FILE *dst, char *str, ...);
static fh_mangle_t * burst_mangle_new(fh_mangle_t *delegate, float evt_rate, int_range_t span_range, float_range_t ber_range, FILE *log);
static fh_mangle_t * burst_drop_new(fh_mangle_t *delegate, float evt_rate, int_range_t span_range, FILE *log);
static fh_error_stream_t * _fh_error_stream_new(fh_stream_t *delegate, fh_mangle_t *mangler);
static int _read(void *ctx, uint8_t *buf, size_t length, int timeout);
static int _write(void *ctx, uint8_t *buf, size_t length);
static void _destroy_adapter(void **self_p);

// fh_stream_t interface
static fh_stream_impl STREAM_IMPL = {.read = &_read, .write = &_write, .destroy_ctx = &_destroy_adapter};

//  Create a new error stream
fh_stream_t *
fh_error_stream_new(fh_stream_t *delegate, float burst_event_rate, FILE *log)
{
    int_range_t span_range = {.min = 2, .max = 4096};
    float_range_t ber_range = {.min = .01, .max = 1};
    fh_mangle_t *mangler1 = burst_drop_new(NULL, burst_event_rate, span_range, log);                    // burst drops
    fh_mangle_t *mangler2 = burst_mangle_new(mangler1, burst_event_rate, span_range, ber_range, log);   // burst bit errors

    return fh_stream_new(_fh_error_stream_new(delegate, mangler2), STREAM_IMPL); // todo add manglers
}

// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
/**
 * Interface for functions that mangle content
 */
typedef int (*mangle_fn)(void *ctx, bool ignore, uint8_t *buf, size_t length);
typedef struct {
    mangle_fn mangle;
    destroy_fn destroy_ctx; // destructor for context, may be NULL
} fh_mangle_impl;

struct _fh_mangle_t {
    fh_mangle_impl *impl;
    void *context;
};

fh_mangle_t *
fh_mangle_new(void *ctx, fh_mangle_impl impl)
{
    fh_mangle_t *self = (fh_mangle_t *)calloc(1, sizeof(fh_mangle_t));
    assert(self);
    self->context = ctx;

    self->impl = (fh_mangle_impl *)calloc(1, sizeof(fh_mangle_impl));
    assert(self->impl);
    self->impl->mangle = impl.mangle;
    self->impl->destroy_ctx = impl.destroy_ctx;

    return self;
}

void
fh_mangle_destroy(fh_mangle_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_mangle_t *self = *self_p;

        // implementation-defined context destructor
        destroy_fn destroy = self->impl->destroy_ctx;
        if (destroy) {
            (*destroy)(&(self->context));
        }

        free(self->impl);
        free(self);
        *self_p = NULL;
    }
}

int
fh_mangle(void *ctx, bool ignore, uint8_t *buf, size_t length)
{
    fh_mangle_t *self = (fh_mangle_t *)ctx;

    return (*(self->impl->mangle))(self->context, ignore, buf, length);
}

// ###############################################################################################
// ###############################################################################################
// # Burst Error Generator
// ###############################################################################################
// ###############################################################################################
/**
 * Implements a burst error mangler
 */

// holds state of one burst error event
typedef struct {
    float ber;                // bit error rate during a burst error //todo actual byte error rate
    uint32_t evt_byte_span;   // duration of event, in bytes
    uint32_t evt_byte_cnt;    // running count of bytes handled during the event
    uint32_t evt_corrupt_cnt; // running count of bytes impacted by the event
} burst_event;

typedef struct {
    fh_mangle_t *delegate;   // next mangler in the chain
    fh_rand_t *random;       // psuedo-random source, isolated for repeatability.

    float evt_rate;          // the rate of burst errors
    float_range_t ber_range; // range of byte error rate during an event
    int_range_t span_range;  // range of bytes spanned by an event

    burst_event *cur_evt;    // the current burst event

    uint32_t byte_cnt;       // running count of bytes outside an event 

    FILE *log;               // tracing outputs written here
} burst_ctx;

static void burst_ctx_destroy_adapter(void **self_p);
static int burst_mangle(void *ctx, bool ignore, uint8_t *buf, size_t length);
static int burst_drop(void *ctx, bool ignore, uint8_t *buf, size_t length);


// initialize a new burst error event
static burst_event *
burst_event_new(burst_ctx *self)
{

    burst_event *evt = (burst_event *)calloc(1, sizeof(burst_event));
    assert(evt);

    evt->evt_byte_span = fh_rand_int(self->random, self->span_range.min, self->span_range.max);
    evt->ber = fh_rand_float(self->random, self->ber_range.min, self->ber_range.min);
    evt->evt_byte_cnt = 0;

    return evt;
}

//  Destroy a burst error event
static void
burst_event_destroy(burst_event **self_p)
{
    assert(self_p);
    if (*self_p) {
        burst_event *self = *self_p;
        free(self);
        *self_p = NULL;
    }
}

static fh_mangle_t *
burst_mangle_new(fh_mangle_t *delegate, float evt_rate, int_range_t span_range, float_range_t ber_range, FILE *log)
{
    fh_mangle_impl impl = {&burst_mangle, &burst_ctx_destroy_adapter};

    burst_ctx *ctx = (burst_ctx *)calloc(1, sizeof(burst_ctx));
    assert(ctx);
    ctx->delegate = delegate;

    ctx->random = fh_rand_new(6845);

    ctx->evt_rate = evt_rate;
    ctx->ber_range = ber_range;
    ctx->span_range = span_range;

    ctx->log = log;
    return fh_mangle_new(ctx, impl);
}

static fh_mangle_t *
burst_drop_new(fh_mangle_t *delegate, float evt_rate, int_range_t span_range, FILE *log)
{
    fh_mangle_impl impl = {&burst_drop, &burst_ctx_destroy_adapter};

    burst_ctx *ctx = (burst_ctx *)calloc(1, sizeof(burst_ctx));
    assert(ctx);
    ctx->delegate = delegate;

    ctx->random = fh_rand_new(93218749);

    ctx->evt_rate = evt_rate;
    ctx->ber_range = (float_range_t){.min = 0, .max = 0};
    ctx->span_range = span_range;

    ctx->log = log;
  
    return fh_mangle_new(ctx, impl);
}

//  Destroy an error context
static void
burst_ctx_destroy(burst_ctx **self_p)
{
    assert(self_p);
    if (*self_p) {
        burst_ctx *self = *self_p;
        fh_mangle_destroy(&(self->delegate));
        fh_rand_destroy(&(self->random));

        if(self->cur_evt)
        {
            burst_event_destroy(&(self->cur_evt));
        }

        free(self);
        *self_p = NULL;
    }
}

//  adapt the destructor
static void
burst_ctx_destroy_adapter(void **self_p)
{
    burst_ctx_destroy((burst_ctx **)self_p);
}


// deturmines when to apply errors to the stream
static bool
in_burst(burst_ctx *self, char *mode)
{
        burst_event *evt = self->cur_evt;

    if (!evt) {
        self->byte_cnt++;
        float roll = fh_rand_float(self->random, 0, 1);
        if (self->evt_rate > roll) {
            evt = burst_event_new(self);
            evt->evt_byte_cnt++; // current byte is in the event
            self->cur_evt = evt;

            logtrace(self->log, "\n*************************\n");
            logtrace(self->log, "roll %f < %f = %d \n", roll, self->evt_rate, (self->evt_rate >= roll));
            logtrace(self->log, "BURST ERROR after %" PRIu32 " bytes: mode:[%s], span: %" PRIu32 ", ber: %f, rate: %f\n", self->byte_cnt,
                   mode, evt->evt_byte_span, evt->ber, (1.0 / (float)self->byte_cnt));
            logtrace(self->log, "*************************\n");
        }
    }
    else {
        // current event over?
        evt->evt_byte_cnt++; 
        if (evt->evt_byte_cnt > evt->evt_byte_span) {
            uint32_t span = evt->evt_byte_cnt;
            uint32_t corrupted = evt->evt_corrupt_cnt;

            burst_event_destroy(&(self->cur_evt));
            self->byte_cnt = 0;

            logtrace(self->log, "\n*************************\n");
            logtrace(self->log, "BURST ERROR END after %" PRIu32 " %s over %" PRIu32 " read bytes, rate: %f\n", corrupted, mode,
                   span, ((float)corrupted / (float)span));
            logtrace(self->log, "*************************\n");
        }
    }

    return self->cur_evt;
}

// transform a stream by occasionally corruption a span of bytes
static int
burst_mangle(void *ctx, bool ignore, uint8_t *buf, size_t length)
{
    burst_ctx *self = (burst_ctx *)ctx;

    // inject bit errors into read buffer
    for (int idx = 0; idx < length; idx++) {
        if (in_burst(self, "MANGLE")) {
            bool bit_error = self->cur_evt->ber >= fh_rand_float(self->random, 0, 1);
            if (bit_error) {
                int bit = fh_rand_int(self->random, 0, 8);
                // uint8_t prev = buf[idx];
                buf[idx] = buf[idx] ^ (1 << bit);
                // logtrace(self->log, "injecting a read error at bit %d of byte %d, replace %#x with %#x\n", bit, idx, prev,
                // buf[idx]);
                fflush(stdout);
                self->cur_evt->evt_corrupt_cnt++;
            }
        }
    }

    if (self->delegate) {
        return fh_mangle(self->delegate, true, buf, length); // todo use ignore
    }
    else {
        return length;
    }
}

// transform a stream by occasionally dropping a span of bytes
static int
burst_drop(void *ctx, bool ignore, uint8_t *buf, size_t length)
{
    burst_ctx *self = (burst_ctx *)ctx;

    // drop from the buffer
    uint8_t passed[length];
    size_t pass_cnt = 0;
    for (int idx = 0; idx < length; idx++) {
        if (in_burst(self, "DROP")) {
            // logtrace(self->log, "dropping a read byte of %#x at %d of %zu\n", buf[idx], idx, length);
            // fflush(stdout);
            self->cur_evt->evt_corrupt_cnt++;
        }
        else {
            passed[pass_cnt++] = buf[idx];
        }
    }

    memcpy(buf, passed, pass_cnt);
    length = pass_cnt;

    if (self->delegate) {
        return fh_mangle(self->delegate, true, buf, length); // todo use ignore
    }
    else {
        return length;
    }
}

// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################

struct _fh_error_stream_t {
    fh_stream_t *delegate;
    fh_mangle_t *mangler;
};

//  Create a new error stream
static fh_error_stream_t *
_fh_error_stream_new(fh_stream_t *delegate, fh_mangle_t *mangler)
{
    fh_error_stream_t *self = (fh_error_stream_t *)calloc(1, sizeof(fh_error_stream_t));
    assert(self);

    self->delegate = delegate;
    self->mangler = mangler;
    return self;
}

//  Destroy an error stream
static void
_fh_error_stream_destroy(fh_error_stream_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_error_stream_t *self = *self_p;
        if (self->mangler) {
            fh_mangle_destroy(&(self->mangler));
        }
        free(self);
        *self_p = NULL;
    }
}

// adaptor for the destructor
static void
_destroy_adapter(void **self_p)
{
    _fh_error_stream_destroy((fh_error_stream_t **)self_p);
}

static int
_read(void *ctx, uint8_t *buf, size_t length, int timeout)
{
    fh_error_stream_t *self = (fh_error_stream_t *)ctx;

    int read = fh_stream_read(self->delegate, buf, length, timeout);

    if (read > 0) {
        // pass through error injection chain
        if (self->mangler) {
            fh_mangle(self->mangler, true, buf, read);
        }
        else {
            return read;
        }
    }

    return read;
}

static int
_write(void *ctx, uint8_t *buf, size_t length)
{
    fh_error_stream_t *self = (fh_error_stream_t *)ctx;
    return fh_stream_write(self->delegate, buf, length);
}
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################
// ###############################################################################################

// trace logging function
static void
logtrace(FILE *dst, char *str, ...)
{
    if (dst) {
        va_list arg;
        va_start(arg, str);
        vfprintf(dst, str, arg);
        va_end(arg);
    }
    else {
        // to /dev/null!
    }
}
