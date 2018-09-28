
// ------------------------------------------------------------------
// main.c
//
// example driver for simulated hit buffer.
//
// ------------------------------------------------------------------

#include "standard_inc.h"
#include "hitbuf.h"
#include "sim_data.h"
#include <pthread.h>


// forwards
static void _dev_null(void *ctx, uint8_t *page, size_t length);
static void _print_page(void *ctx, uint8_t *page, size_t length);
static void _hexdump(FILE *fout, char *desc, void *addr, int len);
static void * _consumer_loop(void *args);



enum format {
  TYPE_0,
  TYPE_1,
  TYPE_2,
  NONE
};

typedef struct {
   int pagecnt;
   uint32_t lastPPS;
   uint32_t lastHitTS;
   enum format curfmt;
} state;


// control buffer size and hit rate
#define BUFFER_SIZE_BITS 19
#define PAGE_SIZE_BITS 11
#define HIT_RATE 100

// control behavior
#define RUN_TIME_SEC 10
#define CONSUMER_ACTION _print_page
// #define CONSUMER_ACTION _dev_null
#define ASYNC_CONSUMER true
//#define ASYNC_CONSUMER false


int
main(int argc, char *argv[])
{
    hitbuf_t *hb = hitbuf_new(BUFFER_SIZE_BITS, PAGE_SIZE_BITS);

    bool threaded = ASYNC_CONSUMER;

    if (threaded) {
        pthread_t pt;
        pthread_create(&pt, NULL, &_consumer_loop, hb);
    }

    printf("loading...\n\n");
    sim_spec spec;
    spec.rps = HIT_RATE;
    sim_data_t *sim =  sim_data_new(hb, spec);
    sim_data_start(sim);

    sleep(RUN_TIME_SEC);

    //sim_data_stop(sim); // todo

    if (!threaded) {
        printf("draining...\n\n");
        // sink sink = {NULL, &_dev_null};
        state st = {0, 0, 0, TYPE_1};
        page_sink sink = {&st, &CONSUMER_ACTION};
        while (hitbuf_pop_page_to(hb, &sink)) {
        }
    }
}


static void *
_consumer_loop(void *args)
{
    hitbuf_t *hb = (hitbuf_t *)args;

    state st = {0, 0, 0, TYPE_1};
    page_sink sink = {&st, &CONSUMER_ACTION};
    while (true) {
      while(hitbuf_pop_page_to(hb, &sink)){}
            sleep(1);

    }
   return NULL;
}

// consume a page without reading content
__attribute__((unused)) void 
_dev_null(void *ctx, uint8_t *page, size_t length)
{
  printf("consume page at %p, length: %zu\n", (void*)page, length);
  return;
}


// consume a page, verifying structure and dumping content to stdout
__attribute__((unused)) void
_print_page(void *ctx, uint8_t *page, size_t length)
{

    state *last = (state *)ctx;

    printf("page %d, length %zu\n", last->pagecnt, length);
    last->pagecnt += 1;

    _hexdump(stdout, "page", page, length);


    uint8_t *end = page + length;
    uint32_t recnum = 0;


    //###############################################
    // 1. consume the required PAGE_INFO, PPSY, PPSS records
    //###############################################
    // PAGE_INFO
    uint32_t info = *((uint32_t *)page);
    uint32_t used_len = (info & DETAIL_MASK) >> 10;
    uint32_t reserved = (info & 0x000003FF);
    printf("record[%d][0x%x] PAGE_INFO used_len: %d, reserved[0x%x]\n", recnum, (info & REC_TYPE_MASK), used_len, reserved);
    page += 4;
    assert(reserved == 0);
    recnum++;

    // PPSY
    uint32_t year = *((uint32_t *)page);
    printf("record[%d][0x%x] PPSY year: %d [0x%x]\n", recnum, (year & REC_TYPE_MASK), (year & DETAIL_MASK), (year & DETAIL_MASK));
    page += 4;
    assert((year & REC_TYPE_MASK) == PPSY_TYPE);
    assert((year & DETAIL_MASK) == 2018);
    recnum++;

    //PPSS
    uint32_t second = *((uint32_t *)page); 
    printf("record[%d][0x%x] PPSS second: %d [0x%x]\n", recnum, (second & REC_TYPE_MASK), (second & DETAIL_MASK), (second & DETAIL_MASK));
    page += 4;
    assert((second & REC_TYPE_MASK) == PPSS_TYPE);
    assert((second & DETAIL_MASK) >= last->lastPPS);
    last->lastPPS = (second & DETAIL_MASK);


     //###############################################
    // 1. consume the remaining records
    //###############################################
    uint32_t ctrl_word;
    while ((page < end) && (ctrl_word = *((uint32_t *)page)) != 0) {

        // consume the control word
        recnum++;
        page += 4;

        if (ctrl_word > 0xDFFFFFFF) {
            // this is a non-hit record
            uint32_t type = (ctrl_word & REC_TYPE_MASK);
            switch (type) {
            case PPSY_TYPE:
                break; // just skip
            case PPSS_TYPE: {
                printf("record[%d][0x%x] PPSS second: %d [0x%x]\n", recnum, (ctrl_word & REC_TYPE_MASK),
                       (ctrl_word & DETAIL_MASK), (ctrl_word & DETAIL_MASK));
                assert((ctrl_word & DETAIL_MASK) >= last->lastPPS);
                last->lastPPS = (ctrl_word & DETAIL_MASK);
                break;
            }
            case FORMAT_SELECTOR_TYPE: {
                uint16_t fmtnum = ((ctrl_word >> 16) & 0x3FF);
                printf("record[%d][0x%x] FORMAT_SELECTOR word[0x%x], data format selector [%d]\n", recnum, type, ctrl_word,
                       fmtnum);
                switch (fmtnum) {
                case 0:
                    last->curfmt = TYPE_0;
                    break;
                case 1:
                    last->curfmt = TYPE_1;
                    break;
                case 2:
                    last->curfmt = TYPE_2;
                    break;
                default:
                    printf("Unknown format %d\n", fmtnum);
                    assert(false);
                    break;
                }

                break;
            }
            case GENERIC_RECORD_TYPE: {
                size_t msglen = (ctrl_word & 0x03FF0000) >> 16;
                char buf[1024];
                buf[0] = (ctrl_word & 0x0000FF00) >> 8;
                buf[1] = (ctrl_word & 0x000000FF);
                int pad = msglen==3? 1:0;
                if(msglen > 4)
                {
                  memcpy(&buf[2], page, msglen-4);
                  int add_msg = (msglen-4);
                  pad = ((add_msg + 3) & ~0x3) - add_msg; // padded to 32-bit word boundary

                  page += (add_msg + pad) ;

                }
                buf[ msglen-2] = 0;
                printf("record[%d][0x%x], GENERIC_MSG , word[0x%x], msg[%zu]: %s, pad[%d]\n", recnum, type, ctrl_word, msglen, (char*)&buf, pad);
                continue;

              }
            }
        }
        else {
            // This is a hit type

          // firstword as timestamp is common to all hit types so far 

          uint32_t utc_sec = ctrl_word + (last->lastPPS * 1000000); // WRONG, treating timestamp as micro-second units instead of 1/16*sysclock
           printf("record[%d][0x%x] HIT time: sec[%d] + offset[%u] = %u\n", recnum, ctrl_word, last->lastPPS, ctrl_word, utc_sec);
          assert(utc_sec >= last->lastHitTS);
          last->lastHitTS = utc_sec;

          switch(last->curfmt)
          {
            case TYPE_0:
              //noop, we've already consumed the timestamp
              break;
            case TYPE_1: {
              uint32_t *cur_word = (uint32_t*)(page);
              page += 4;
              uint16_t adc_count = ((*cur_word) >> 28) & 0xF;
              uint16_t tot = ((*cur_word) >> 16) & 0xFFF;
              printf("   adc_count: %d, tot: %d\n", adc_count, tot);
              // adc values starting mid-word
              // note: this loop ends on a word boundary
              for (int i = 0; i < adc_count; i++) {
                  if ((i & 1) == 0) {
                      uint16_t adcidx = ((*cur_word) >> 12) & 0xF ;
                      uint16_t adc = (*cur_word) & 0xFFF;
                      printf("   adc[%d]:%d\n", adcidx, adc);
                  }
                  else {
                      cur_word = (uint32_t*)page;
                      page += 4;
                      uint16_t adcidx = ((*cur_word) >> 28) & 0xF;
                      uint16_t adc = ((*cur_word) >> 16) & 0xFFF;
                      printf("   adc[%d]:%d\n", adcidx, adc);
                  }
              }

              break;
            }
            case TYPE_2:
            printf("todo: implement format %d\n", last->curfmt);
               printf("todo: implement\n");
              assert(false);
              page = end;
              break;
            case NONE:
              printf("No current format.\n");
              assert(false);
              page = end;
              break;
          }
        }

    }

    if(page != end)
    {
      printf("XXX PAGE padded by %td\n", (end-page) );
    }
}



// generate a hex dump of arbitrary memory to a stream
static void
_hexdump(FILE *fout, char *desc, void *addr, int len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *)addr;

    // Output description if given.
    if (desc != NULL) fprintf(fout, "%s:\n", desc);

    if (len == 0) {
        fprintf(fout, "  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        fprintf(fout, "  NEGATIVE LENGTH: %i\n", len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0) fprintf(fout, "  %s\n", buff);

            // Output the offset.
            fprintf(fout, "  %04x ", i);
        }

        // Now the hex code for the specific character.
        fprintf(fout, " %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        fprintf(fout, "   ");
        i++;
    }

    // And print the final ASCII bit.
    fprintf(fout, "  %s\n", buff);
}






