/**
 * pagebuf.h
 *
 * A buffer of pages. 
 */
#ifndef PAGEBUF_INCLUDED
#define PAGEBUF_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "ringbuf.h" // bring in page_sink type


typedef struct _pagebuf_t pagebuf_t;

// holds diagnostic accounting of page buffer
typedef struct {
    uint32_t pushed;
    uint32_t popped;
    uint32_t overflows;
} pagebuf_stats;

// create a new page buffer
// page_bits : size of a page specified as a power of 2
// size_bits : size of buffer specified as a power of 2hitbuf_t *
pagebuf_t *
pagebuf_new(int size_bits, int page_bits);

// destroy a hit buffer
void
pagebuf_destroy(pagebuf_t **self_p);

// read a page if available, copying data into a target location
// returns false if no pages are avalable
bool
pagebuf_pop_page(pagebuf_t *self, uint8_t *dst);

// read a page if available, passing a ponter to the page to a callback function.
// the page content is stable over the scope of the callback
// sink : callback invoked with the page data
// returns false if no pages are avalable
bool
pagebuf_pop_page_to(pagebuf_t *self, page_sink *sink);

// copy a page of data into the buffer
bool
pagebuf_push_page(pagebuf_t *self, uint8_t *page, size_t len);

// get stats
void
pagebuf_get_stats(pagebuf_t *self, pagebuf_stats*);



#ifdef __cplusplus
}
#endif

#endif
