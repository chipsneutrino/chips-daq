
/**
 * pagebuf.c
 *
 * Prototype a page buffer
 *
 * NOTE: multi-byte fields defined in the page structure, including the control
 *       word, are stored in native endian form. Non-local recipients of page
 *       data will require knowledge of the source endianess to digest the
 *       page contents
 *
 * TODO: Implement an overflow policy
 */

#include "standard_inc.h"

#include "pagebuf.h"
#include "ringbuf.h"
#include <pthread.h>

static inline void _lock_ring(pagebuf_t *self);
static inline void _unlock_ring(pagebuf_t *self);
static inline void _copy_to(void *ctx, uint8_t *data, size_t length);
static inline void _dev_null(void *ctx, uint8_t *data, size_t length);

// instance data
struct _pagebuf_t {
    ringbuf_t *buf;       // ring buffer of pages
    pthread_mutex_t lock; // lock for mltithreaded read/write of the ring
    uint32_t page_sz;     // size of a page
    pagebuf_stats stats;  // diagnostic stats
};

// create a new page buffer
// page_bits : size of a page specified as a power of 2
// size_bits : size of buffer specified as a power of 2hitbuf_t *
pagebuf_t *
pagebuf_new(int size_bits, int page_bits)
{
    pagebuf_t *self = (pagebuf_t *)calloc(1, sizeof(pagebuf_t));
    assert(self);

    assert(pthread_mutex_init(&(self->lock), NULL) == 0);

    self->page_sz = 1 << page_bits;
    self->buf = ringbuf_new(size_bits, page_bits);
    return self;
}

// destroy a hit buffer
void
pagebuf_destroy(pagebuf_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        pagebuf_t *self = *self_p;
        ringbuf_destroy(&(self->buf));
        pthread_mutex_destroy(&(self->lock));
        *self_p = NULL;
    }
}

// read a page if available, copying data into a target location
// returns false if no pages are avalable
bool
pagebuf_pop_page(pagebuf_t *self, uint8_t *dst)
{
    page_sink sink = {dst, &_copy_to};
    return ringbuf_pop_to(self->buf, &sink);
}

// read a page if available, passing a ponter to the page to a callback function.
// the page content is stable over the scope of the callback
// sink : callback invoked with the page data
// returns false if no pages are avalable
bool
pagebuf_pop_page_to(pagebuf_t *self, page_sink *sink)
{
    _lock_ring(self);
    bool status = ringbuf_pop_to(self->buf, sink);
    _unlock_ring(self);

    if (status) {
        self->stats.popped++;
    }
    return status;
}

// copy a page of data into the buffer
bool
pagebuf_push_page(pagebuf_t *self, uint8_t *page, size_t len)
{
    assert(len <= self->page_sz);

    bool overflow = false;

    _lock_ring(self);

    uint8_t *next_page = ringbuf_allocate(self->buf);

    // todo: define an overflow strategy
    if (next_page == NULL) {
        overflow = true;
        // printf("OVERFLOW!\n");
        self->stats.overflows++;

        // pop a page to /dev/null
        page_sink sink = {NULL, &_dev_null};
        if (!ringbuf_pop_to(self->buf, &sink)) {
            // illogical since we are commiting a page at each allocation
            assert(false);
        }

        next_page = ringbuf_allocate(self->buf);
        if (next_page == NULL) {
            // illogical since we just popped a page
            assert(false);
        }
    }

    memcpy(next_page, page, len);
    memset(next_page + len, 0, (self->page_sz - len));

    ringbuf_commit(self->buf);
    self->stats.pushed++;

    _unlock_ring(self);

    return overflow;
}

// get stats
void
pagebuf_get_stats(pagebuf_t *self, pagebuf_stats *stats)
{
    _lock_ring(self); // abuse the ring lock for a memory fence and consitency lock
    memcpy(stats, &(self->stats), sizeof(pagebuf_stats));
    _unlock_ring(self);
}

static inline void
_lock_ring(pagebuf_t *self)
{
    pthread_mutex_lock(&(self->lock));
}

static inline void
_unlock_ring(pagebuf_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

static inline void
_copy_to(void *ctx, uint8_t *data, size_t length)
{
    memcpy(ctx, data, length);
}

static inline void
_dev_null(void *ctx, uint8_t *data, size_t length)
{
    // noop
}
