/**
 * ringbuf.h
 *
 * ring buffer like structure with page-oriented interface and 
 * functionality to allocate pages for writing without making
 * them available for reading.
 */
#ifndef RINGBUF_INCLUDED
#define RINGBUF_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _ringbuf_t ringbuf_t;

// callback object for data pop
// NOTE: data item is only valid during the scope of the callback
typedef void (*receive_fn)(void *ctx, uint8_t *page, size_t length);
typedef struct {
void *ctx;
receive_fn callback;
} page_sink;


// create a new ring buffer
// size_bits : size of buffer specified as a power of 2
// page_bits : size of a page specified as a power of 2
ringbuf_t *
ringbuf_new(uint16_t size_bits, uint16_t page_bits);

// destroy a ring buffer
void
ringbuf_destroy(ringbuf_t **self_p);

// num pages free for allocation
uint32_t
ringbuf_num_free(ringbuf_t *self);

// num pages allocated for writing
uint32_t
ringbuf_num_allocated(ringbuf_t *self);

// num pages commited
uint32_t
ringbuf_num_committed(ringbuf_t *self);

// true if ringbuf is fully allocated
bool
ringbuf_full(ringbuf_t *self);

// true if a free page is available for allocation
bool
ringbuf_free(ringbuf_t *self);

// true if an allocated page is available for commit
bool
ringbuf_allocated(ringbuf_t *self);

// true if a commited page is available for read
bool
ringbuf_committed(ringbuf_t *self);

// allocate the next page for writing.
// returns NULL if buffer is full, otherwise pointer to start
// of the allocated page.
uint8_t *
ringbuf_allocate(ringbuf_t *self);

// commit an allocated page
// returns false if there was not an allocated page available
bool
ringbuf_commit(ringbuf_t *self);

// read a page if available, copying data into a target location
// returns false if no pages are avalable
bool
ringbuf_pop(ringbuf_t *self, uint8_t *dst);

// read a page if available, passing a ponter to the page to a callback function
// sink : callback invoked with the page data
// returns false if no pages are avalable
bool
ringbuf_pop_to(ringbuf_t *self, page_sink *sink);

// print details of the ringbuf
void
ringbuf_info(ringbuf_t *self, FILE *out, char *prefix);

#ifdef __cplusplus
}
#endif

#endif
