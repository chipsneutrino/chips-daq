
/**
 * hitbuf.h
 *
 * Prototype a microdaq hit buffer.
 *
 * NOTE: multi-byte fields defined in the page structure, including the control
 *       word, are stored in native endian form. Non-local recipients of page
 *       data will require knowledge of the source endianess to digest the
 *       page contents
 *
 * TODO: Define meaningful MAX record sizes
 * TODO: Implement an overflow policy
 * TODO: Capture page utilization on commit/emit on pop.
 */

//NOTE: added a provisional "PAGE_INFO word" as the first word of each page
 // bits 26-31 : 0101 01
 // bits 10- 25: number of utilized bytes on the page (populated when page is committed)
 // bits 0-9 : zero-filled, reserved for collating clients to identify page source.

// FROM udaq2.0 hit_buffer.c...
/*
 * There are three types of data stored in the hit buffer:
 * (1) pps time markers (second and year)
 * (2) data format selector
 * (3) triggered hit data (interpreted according to latest data format selector)
 * (4) generic message (e.g. slow control log)
 *
 * Every hit buffer page has to start with pps markers to define year and
 * second-of-year, as well as a beginning data format selector.  Additional
 * markers and selectors are added as needed within the data stream of that
 * page.
 *
 * Data are stored as a sequence of 32-bit words.  Some words have several
 * quantities or flags stuffed into bit fields.
 *
 * N.B. we maintain 4-byte alignment for 32-bit quantities
 *
 * We The highest order six bits in the first word of a data chunk are used to
 * distinguish data types.
 *
 * data format selector:
 * word_1: N.B. this can be repeated to change format as often as needed,
 *              applying to subsequent hits until changed again
 * bits 26-31: 1111 11
 * bits 16-25: hit data format subtype (i.e. which format will we use)
 * bits 0-15: format detail bits (meaning depends on which format is selected)
 *
 * hit data type 0: time stamp only
 *                  format detail bits: not used
 * word_1: hit time, relative to last PPS marker, units 1/(16*sysclk)
 *       ==> must be less than 0xe0000000, otherwise is a different data type
 *
 * hit data type 1: time stamp plus ToT with dynamic choice of adc's
 *                  format detail bits: not used
 * word_1: hit time, relative to last PPS marker, units 1/(16*sysclk)
 *       ==> must be less than 0xe0000000, otherwise is a different data type
 * word_2: ToT and adc info, bit-packed
 *       bits 28-31: number of adc values saved
 *       bits 16-27: time over threshold, units 1/(16*sysclk)
 *       bits 12-15: index (i.e. which adc) for first adc value
 *       bits 0-11: first adc value
 * word_3...: remaining adc measurements, up to 2 per word
 *       bits 28-31: index
 *       bits 16-27: adc value
 *       bits 12-15: index
 *       bits 0-11: adc value
 *
 * hit data type 2: time stamp plus fixed number of adc values (no ToT)
 *                  format detail bits 0-3: how many adc values
 * word_1: hit time, relative to last PPS marker, units 1/(16*sysclk)
 *       ==> must be less than 0xe0000000, otherwise is a different data type
 * word_2...: adc measurements, up to 2 per word
 *       bits 28-31: index (i.e. which adc)
 *       bits 16-27: adc value
 *       bits 12-15: index (i.e. which adc)
 *       bits 0-11: adc value
 *
 * PPS year marker format:
 * word_1: type bits and year number
 *       bits 26-31: 1111 10
 *       bits 0-15: year number (2xxx)
 * (will be included before the PPS second marker in buffer page, but will not
 *  be inserted again for a given buffer page fill unless the year changes)
 *
 * PPS second marker format:
 * word_1: type bits and second of year
 *       bits 26-31: 1111 00
 *       bits 0-25: second of year
 *       (seconds start at zero, computed as second_of_day+(day-1)*24*60*60)
 *
 * Generic message (internal message structure not defined here)
 * word_1: type bits and start of message
 *        bits 26-31: 1110 00
 *        bits 16-25: number of bytes in message (including this header)
 *        bits 0-15: first two bytes of message
 * word_2...:  remaining message data padded to multiple of 4 bytes
 *        (if message contains only two bytes then word_1 stands on its own)
 */

#include "standard_inc.h"
#include "hitbuf.h"
#include "ringbuf.h"
#include <pthread.h>

// todo define rational sizing constants
// sizing constants
#define MAX_RECORD_SIZE 256 // arbitrary
#define PPSY_RECORD_SIZE 4
#define PPSS_RECORD_SIZE 4
#define FORMAT_SELECTOR_RECORD_SIZE 4
#define MIN_PAGE_SIZE (MAX_RECORD_SIZE + PPSY_RECORD_SIZE + PPSS_RECORD_SIZE + FORMAT_SELECTOR_RECORD_SIZE)

// hit format identifiers
#define HIT_FMT_0 0x0
#define HIT_FMT_1 0x1
#define HIT_FMT_2 0x2

// sentinel values for pre-initialzed state
#define HIT_FMT_NONE 0xFFFFFFFF // sentinel value
#define PPSS_NONE 0xFFFFFFFF    // sentinel value
#define PPSY_NONE 0xFFFFFFFF    // sentinel value

// forwards
static inline void _lock_ring(hitbuf_t *self);
static inline void _unlock_ring(hitbuf_t *self);
static inline void _write_word(hitbuf_t *self, uint32_t word);
static bool _next_page(hitbuf_t *self);
static inline void _ensure_available(hitbuf_t *self, size_t size);
static inline void _ensure_PPS(hitbuf_t *self, uint32_t year, uint32_t second);
static inline void _ensure_format(hitbuf_t *self, uint32_t fmt);
static inline void _emit_page_info(hitbuf_t *self);
static inline void _update_page_info(hitbuf_t *self);
static inline void _emit_PPSY(hitbuf_t *self);
static inline void _emit_PPSS(hitbuf_t *self);
static inline void _emit_fmt_sel(hitbuf_t *self);
static inline void _truncate_page_to(void *ctx, uint8_t *data, size_t length);
static inline void _copy_to(void *ctx, uint8_t *data, size_t length);
static inline void _dev_null(void *ctx, uint8_t *data, size_t length);

// instance data
struct _hitbuf_t {
    ringbuf_t *buf;        // ring buffer of pages
    pthread_mutex_t lock;  // lock for mltithreaded read/write of the ring
    uint32_t page_sz;      // size of a page
    uint8_t *cur_page;     // pointer to the start of the current allocated page
    uint8_t *cur_page_idx; // pointer to next available byte on the current page
    uint32_t cur_fmt_sel;  // the last hit format selector emitted to the buffer
    uint32_t cur_ppsy;     // the last pps year value emiited to the buffer
    uint32_t cur_ppss;     // the last pps second value emiited to the buffer
};

// create a new hit buffer
// page_bits : size of a page specified as a power of 2
// size_bits : size of buffer specified as a power of 2hitbuf_t *
hitbuf_t *
hitbuf_new(int size_bits, int page_bits)
{
    assert((1 << page_bits) > MIN_PAGE_SIZE);

    hitbuf_t *self = (hitbuf_t *)calloc(1, sizeof(hitbuf_t));
    assert(self);

    assert(pthread_mutex_init(&(self->lock), NULL) == 0);

    self->page_sz = 1 << page_bits;
    self->buf = ringbuf_new(size_bits, page_bits);
    self->cur_page = ringbuf_allocate(self->buf);
    self->cur_page_idx = self->cur_page;

    self->cur_ppsy = PPSS_NONE;
    self->cur_ppss = PPSY_NONE;
    self->cur_fmt_sel = HIT_FMT_NONE;

    _emit_page_info(self);

    return self;
}

// destroy a hit buffer
void
hitbuf_destroy(hitbuf_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        hitbuf_t *self = *self_p;
        ringbuf_destroy(&(self->buf));
        pthread_mutex_destroy(&(self->lock));
        free(self);
        *self_p = NULL;
    }
}

// read a page if available, copying data into a target location
// returns false if no pages are avalable
bool
hitbuf_pop_page(hitbuf_t *self, uint8_t *dst)
{
    page_sink sink = {dst, &_copy_to};
    bool status = ringbuf_pop_to(self->buf, &sink);

    return status;
}

// read a page if available, passing a ponter to the page to a callback function.
// the page content is stable over the scope of the callback
// sink : callback invoked with the page data
// returns false if no pages are avalable
bool
hitbuf_pop_page_to(hitbuf_t *self, page_sink *sink)
{
    _lock_ring(self);
    bool status = ringbuf_pop_to(self->buf, sink);
    _unlock_ring(self);

    return status;
}

// read a page if available, passing a ponter to the page _as well
// as the page leng trucated to the used page size_ to a callback function,
// the page content is stable over the scope of the callback
// sink : callback invoked with the page data
// returns false if no pages are avalable
bool
hitbuf_pop_trunc_page_to(hitbuf_t *self, page_sink *sink)
{
     _lock_ring(self);
    bool status = ringbuf_pop_to(self->buf, &(page_sink){sink, &_truncate_page_to});
    _unlock_ring(self);

    return status;
}


// Add a type 0 trigger record to the buffer
// type 0: timestamp only
void
hit_buf_add_trig_t0(hitbuf_t *self, uint32_t year, uint32_t sec, uint32_t offset_time)
{
    assert(offset_time < 0xE0000000);

    _ensure_PPS(self, year, sec);

    _ensure_format(self, HIT_FMT_0);

    _ensure_available(self, 4);

    _write_word(self, offset_time); // NOTE: native endianess

}

// Add a type 1 trigger record to the buffer
// type 1: (TOT + dynamic ADC count)
void
hit_buf_add_trig_t1(hitbuf_t *self, uint32_t year, uint32_t sec, uint32_t offset_time, uint16_t tot, uint16_t *adc,
                    uint8_t adc_count)
{
    assert(offset_time < 0xE0000000);
    assert(tot < 4096);      // limited to 12 bits
    assert(adc_count <= 15); // limited to 4 bits

    // calculate record size
    size_t size = 8;             // timestamp, ADC count, TOT, ADC#1
    size += (adc_count - 1) * 2; // remaining ADC
    size = (size + 3) & ~0x3;    // pad to 32-bit word boundary

    _ensure_PPS(self, year, sec);

    _ensure_format(self, HIT_FMT_1);

    _ensure_available(self, size);

    // write record
    // printf("Wrote timestamp 0x%x\n", offset_time);
    _write_word(self, offset_time); // NOTE: native endianess

    uint32_t *cur_word = (uint32_t *)(self->cur_page_idx);
    self->cur_page_idx += 4;

    *cur_word = adc_count << 28;         // NOTE: native endianess
    *cur_word = *cur_word | (tot << 16); // NOTE: native endianess

    // adc values starting mid-word
    for (int i = 0; i < adc_count; i++) {
        if ((i & 1) == 0) {
            *cur_word = (*cur_word) | (*adc); // NOTE: native endianess
        }
        else {
            cur_word = (uint32_t *)(self->cur_page_idx); // NOTE: native endianess
            self->cur_page_idx += 4;
            *cur_word = (*adc) << 16;
        }
        adc++;
    }
}

// Add a type 0 trigger record to the buffer
// type 2: (static adc_count)
void
hit_buf_add_trig_t2(hitbuf_t *self, uint32_t offset_time, uint16_t *adc, uint8_t adc_count)
{
    // todo
}

// add a generic record to the buffer
void
hit_buf_add_generic(hitbuf_t *self, uint8_t *data, size_t len)
{

    size_t msglen; // the number of bytes containing the message
    size_t padlen; // the number of padding bytes added beyod control word
    uint32_t ctrl_word; // the control word
    uint8_t *copydata;  // pointer to data in exess of the 2 bytes encoded in the control word
    size_t copylen;     // the number of data bytes remaining after encoding 2 bytes into the control word

    switch (len) {
    case 0: {
        padlen = 0;
        msglen = 2 + len;
        ctrl_word = 0;
        copydata = 0;
        copylen = 0;
        break;
    }
    case 1: {
        padlen = 0;
        msglen = 2 + len;
        // load data byes following native endian
        ctrl_word = (*data) << 8;
        copydata = 0;
        copylen = 0;
        break;
    }
    case 2: {
        padlen = 0;
        msglen = 2 + len;

        // load data bytes following native endian
        ctrl_word = (*data) << 8;
        ctrl_word |= *(data + 1);
        copydata = 0;
        copylen = 0;
        break;
    }
    default: {
        msglen = len + 2;
        padlen = ((msglen + 3) & ~0x3) - msglen; // pad to 32-bit word boundary
        // load data byes following native endian
        ctrl_word = (*data) << 8;
        ctrl_word |= *(data + 1);
        copydata = data + 2;
        copylen = len - 2;
        break;
    }
    }

    _ensure_available(self, 4 + copylen + padlen);

    ctrl_word |= (GENERIC_RECORD_TYPE | (msglen << 16));
    _write_word(self, ctrl_word); // NOTE: native endianess

    // write data in excess of the 2 bytes encoded in control word
    if (copylen > 0) {
        memcpy((self->cur_page_idx), copydata, copylen);
        memset((self->cur_page_idx + copylen), 0, padlen);

        self->cur_page_idx += copylen + padlen;
    }

}

static inline void
_lock_ring(hitbuf_t *self)
{
    pthread_mutex_lock(&(self->lock));
}

static inline void
_unlock_ring(hitbuf_t *self)
{
    pthread_mutex_unlock(&(self->lock));
}

// caller responsible to ensure available space
static inline void
_write_word(hitbuf_t *self, uint32_t word)
{
    *((uint32_t *)(self->cur_page_idx)) = word;
    self->cur_page_idx += 4;
}

// allocates and initializes a new page
// returns true if allocation required overwriting
// an existion page.
static bool
_next_page(hitbuf_t *self)
{
    bool overflow = false;

    _lock_ring(self);
    uint8_t *next_page = ringbuf_allocate(self->buf);
    _unlock_ring(self);

    // todo: define an overflow strategy
    if (next_page == NULL) {
        overflow = true;
        // printf("OVERFLOW!\n");

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
    self->cur_page = next_page;
    self->cur_page_idx = self->cur_page;

    // emit std page header records
    _emit_page_info(self);
    _emit_PPSY(self);
    _emit_PPSS(self);
    _emit_fmt_sel(self);

    return overflow;
}

// ensure that a record of a particular size will fit on the current page,
// rolling to a new page if required.
static inline void
_ensure_available(hitbuf_t *self, size_t size)
{
    assert(size <= (self->page_sz - 8)); // allow for auto-inserted PPSY/PPSS records

    // roll page if required
    int remaining = self->page_sz - (self->cur_page_idx - self->cur_page);
    if (remaining < size) {

        // update the page status word
        _update_page_info(self);

        // zero-fill unused page space
        uint8_t *end = self->cur_page + self->page_sz;
        while (self->cur_page_idx < end) {
            *(self->cur_page_idx++) = 0;
        }
        
        assert(ringbuf_commit(self->buf));

        bool overflow = _next_page(self);

        // emit a generic record to indicate overflow
        if (overflow) {
            hit_buf_add_generic(self, (uint8_t *)"OVERFLOW", 8);

            // NOTE: This fails if a MAX size record is in play since
            // we utilized page space for the overflow message. Fail
            // for now
            remaining = self->page_sz - (self->cur_page_idx - self->cur_page);
            assert(remaining >= size);
        }
    }
}

// ensures that the supplied pps sec/year has been emitted to the buffer
static inline void
_ensure_PPS(hitbuf_t *self, uint32_t year, uint32_t second)
{
    if (self->cur_ppsy != year) {
        _ensure_available(self, 8);
        self->cur_ppsy = year;
        _emit_PPSY(self);
        self->cur_ppss = second;
        _emit_PPSS(self);
    }
    else if (self->cur_ppss != second) {
        _ensure_available(self, 4);
        self->cur_ppss = second;
        _emit_PPSS(self);
    }
}

// ensures that the supplied format is the current format of the buffer
static inline void
_ensure_format(hitbuf_t *self, uint32_t fmt)
{
    if (self->cur_fmt_sel != fmt) {
        self->cur_fmt_sel = fmt;
        _ensure_available(self, 4);
        _emit_fmt_sel(self);
    }
}

// emit the page status word.(with length=0)
static inline void
_emit_page_info(hitbuf_t *self)
{
    uint32_t ctrl_word = PAGE_INFO_TYPE;
    _write_word(self, ctrl_word);
}

// called prior to commiting page to poulate
// the page status record
static inline void
_update_page_info(hitbuf_t *self)
{
    int length = self->cur_page_idx - self->cur_page;
    uint32_t ctrl_word = (length << 10) | PAGE_INFO_TYPE;

    // NOTE: writing directly to first word of page
    *((uint32_t *)(self->cur_page)) = ctrl_word;
}

static inline void
_emit_PPSY(hitbuf_t *self)
{
    assert(self->cur_ppsy < 0x0000FFFF);

    uint32_t ctrl_word = self->cur_ppsy | PPSY_TYPE;
    _write_word(self, ctrl_word);
}

static inline void
_emit_PPSS(hitbuf_t *self)
{
    assert(self->cur_ppss < 0x03FFFFFF);

    uint32_t ctrl_word = self->cur_ppss | PPSS_TYPE;
    _write_word(self, ctrl_word);
}

static inline void
_emit_fmt_sel(hitbuf_t *self)
{
    assert(self->cur_fmt_sel < 0x000003FF);

    uint32_t ctrl_word = (self->cur_fmt_sel << 16) | FORMAT_SELECTOR_TYPE;
    _write_word(self, ctrl_word);
}


// forward a page to a delegate, replacing the full page length with
// the used length (read from the page info field). 
__attribute__((unused))
static inline void
_truncate_page_to(void *ctx, uint8_t *data, size_t length)
{
    // read the used lenth from the page info word
    uint32_t ctrl_word = *(uint32_t*)data;
    uint32_t used_len = (ctrl_word & DETAIL_MASK) >> 10;

    printf("truncate %zu to %d\n", length, used_len);

    assert(used_len <= length);

    // forward to the delegate with the truncated length
    page_sink *delegate = (page_sink*)ctx;
    (*(delegate->callback))(delegate->ctx, data, used_len); 
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
