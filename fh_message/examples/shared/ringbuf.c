
/**
 * ringbuf.c
 *
 * ring buffer like structure with page-oriented interface and 
 * functionality to allocate pages for writing without making
 * them available for reading.
 */

#include "standard_inc.h"
#include "ringbuf.h"



// instance data
struct _ringbuf_t {
    uint8_t *buf;      // allocated storage
    uint32_t head;     // virtual write index  (byte-oriented)
    uint32_t commit;   // virtual commit index (byte-oriented)
    uint32_t tail;     // virtual read index (byte-oriented)
    uint32_t size;     // size of buffer in bytes
    uint32_t num_page; // size of buffer in pages
    uint32_t page_sz;  // page size
    uint32_t mask;     // mask to resolve virtual indexes to pysical addresses
};

// forwards
static inline uint8_t * _ringbuf_resolve(ringbuf_t *self, uint32_t vidx);
static inline void _copy_to(void *ctx, uint8_t *data, size_t length);

// create a new ring buffer
// size_bits : size of buffer specified as a power of 2
// page_bits : size of a page specified as a power of 2
ringbuf_t *
ringbuf_new(uint16_t size_bits, uint16_t page_bits)
{
    assert(page_bits <= size_bits);
    assert( size_bits <=   31);  // high bit of indexes function as a flag bit

    ringbuf_t *self = (ringbuf_t *)calloc(1,sizeof(ringbuf_t));
    assert(self);

    self->size = 1 << size_bits;
    self->page_sz = 1 << page_bits;
    self->num_page = 1 << (size_bits - page_bits);
    self->mask = self->size - 1;
   
    self->buf = (uint8_t*)calloc(1,self->size);

    self->head = 0;
    self->commit = 0;
    self->tail = 0;

    return self;
}

// destroy a ring buffer
void
ringbuf_destroy(ringbuf_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        ringbuf_t *self = *self_p;
        free(self->buf);
        free(self);
        *self_p = NULL;
    }
}

// num pages free for allocation
uint32_t
ringbuf_num_free(ringbuf_t *self)
{
    return self->num_page - ((uint32_t)(self->head - self->tail) / self->page_sz);
}

// num pages allocated for writing
uint32_t
ringbuf_num_allocated(ringbuf_t *self)
{
    return (uint32_t)(self->head - self->commit) / self->page_sz;
}

// num pages commited
uint32_t
ringbuf_num_committed(ringbuf_t *self)
{
    return (uint32_t)(self->commit - self->tail) / self->page_sz;
}

// true if ringbuf is fully allocated
bool
ringbuf_full(ringbuf_t *self)
{
    return (uint32_t)(self->head - self->tail) == self->size; // byte-oriented calc
}

// true if a free page is available for allocation
bool
ringbuf_free(ringbuf_t *self)
{
    return ((uint32_t)(self->head - self->tail) / self->page_sz) < self->num_page;
}

// true if an allocated page is available for commit
bool
ringbuf_allocated(ringbuf_t *self)
{
    return self->head != self->commit;
}

// true if a commited page is available for read
bool
ringbuf_committed(ringbuf_t *self)
{
    return self->commit != self->tail;
}

// allocate the next page for writing.
// returns NULL if buffer is full, otherwise pointer to start
// of the allocated page.
uint8_t *
ringbuf_allocate(ringbuf_t *self)
{
    if (ringbuf_free(self)) {
        uint8_t *ret = _ringbuf_resolve(self, self->head);
        self->head += self->page_sz; // note virtual w/wrapping
        //ringbuf_info(self, stdout, "allocated");

        return ret;
    }
    else {
        return NULL;
    }
}

// commit an allocated page
bool
ringbuf_commit(ringbuf_t *self)
{
    if (ringbuf_allocated(self)) {
        self->commit += self->page_sz; // note virtual w/wrapping
         //ringbuf_info(self, stdout, "commited");
        return true;
    }
    else {
        return false;
    }
}

// read a page if available, copying data into a target location
// returns false if no pages are avalable
bool
ringbuf_pop(ringbuf_t *self, uint8_t *dst)
{
    page_sink sink = {dst, &_copy_to};
    bool status = ringbuf_pop_to(self, &sink);
    return status;
}

// read a page if available, passing a ponter to the page to a callback function
// sink : callback invoked with the page data
// returns false if no pages are avalable
bool
ringbuf_pop_to(ringbuf_t *self, page_sink *sink)
{
    if (ringbuf_committed(self)) {
        (*(sink->callback))(sink->ctx, _ringbuf_resolve(self, self->tail), self->page_sz);
        self->tail += self->page_sz;
         //ringbuf_info(self, stdout, "popped");
        return true;
    }
    else {
        // buffer empty
        return false;
    }
}

// print details of the ringbuf
void
ringbuf_info(ringbuf_t *self, FILE *out, char *prefix)
{
    fprintf(out, "[%s] ringbuf: size(bytes: %d, pagesz: %d, pages: %d), free: %d, allocated: %d, committed: %d, tail: "
                 "%u, commit: %u, head: %u\n",
            prefix, self->size, self->page_sz, self->num_page, ringbuf_num_free(self), ringbuf_num_allocated(self),
            ringbuf_num_committed(self), self->tail, self->commit, self->head);
}


// resolve a virtual index to physical
static inline uint8_t *
_ringbuf_resolve(ringbuf_t *self, uint32_t vidx)
{
    return self->buf + (vidx & self->mask);
}


// implements a copy callback
static inline void
_copy_to(void *ctx, uint8_t *data, size_t length)
{
    memcpy(ctx, data, length);
}
