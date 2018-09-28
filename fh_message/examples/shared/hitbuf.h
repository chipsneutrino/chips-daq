/**
 * hitbuf.h
 *
 * Prototype a microdaq hit buffer. 
 */
#ifndef HITBUF_INCLUDED
#define HITBUF_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "ringbuf.h" // bring in page_sink type


typedef struct _hitbuf_t hitbuf_t;


#define REC_TYPE_MASK 0xFC000000  // masks the record type from a control word
#define DETAIL_MASK 0x03FFFFFF    // masks the detail field from a control word (precomputed ~REC_TYPE_MASK)

#define PAGE_INFO_TYPE 0x54000000         // PAGE_INFO type (pre-shifted to the high 6 bits)
#define PPSY_TYPE 0xF8000000              // PPSY type (pre-shifted to the high 6 bits)
#define PPSS_TYPE 0xF0000000              // PPSS type (pre-shifted to the high 6 bits)
#define FORMAT_SELECTOR_TYPE 0xFC000000   // FORMAT_SELECTOR type (pre-shifted to the hig 6 bits)
#define GENERIC_RECORD_TYPE 0xE0000000    // GENERIC RECORD type (pre-shifted to the hig 6 bits)


// create a new hit buffer
// size_bits : size of buffer specified as a power of 2
// page_bits : size of a page specified as a power of 2
hitbuf_t *
hitbuf_new(int size_bits, int page_bits);

// destroy a hit buffer
void
hitbuf_destroy(hitbuf_t **self_p);

// read a page if available, copying data into a target location
// returns false if no pages are avalable
bool
hitbuf_pop_page(hitbuf_t *self, uint8_t *dst);

// read a page if available, passing a ponter to the page to a callback function.
// the page content is stable over the scope of the callback
// sink : callback invoked with the page data
// returns false if no pages are avalable
bool
hitbuf_pop_page_to(hitbuf_t *self, page_sink *sink);

// read a page if available, passing a ponter to the page _as well
// as the page leng trucated to the used page size_ to a callback function,
// the page content is stable over the scope of the callback
// sink : callback invoked with the page data
// returns false if no pages are avalable
bool
hitbuf_pop_trunc_page_to(hitbuf_t *self, page_sink *sink);

// Add a type 0 trigger record to the buffer
// type 0: timestamp only
void
hit_buf_add_trig_t0(hitbuf_t *self, uint32_t year, uint32_t sec, uint32_t offset_time);

// Add a type 1 trigger record to the buffer
// type 1: (TOT + dynamic ADC count)
void
hit_buf_add_trig_t1(hitbuf_t *self, uint32_t year, uint32_t sec, uint32_t offset_time, uint16_t tot, uint16_t *adc, uint8_t adc_count);

// Add a type 0 trigger record to the buffer
// type 2: (static adc_count)
void
hit_buf_add_trig_t2(hitbuf_t *self, uint32_t offset_time, uint16_t *adc, uint8_t adc_count);

// add a generic record to the buffer
void
hit_buf_add_generic(hitbuf_t *self, uint8_t *data, size_t len);





#ifdef __cplusplus
}
#endif

#endif
