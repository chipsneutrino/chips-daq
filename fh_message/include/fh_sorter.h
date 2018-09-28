/**
 * fh_sorter.h
 *
 * Implements an N-to-1 sorter useful for combining N channels of
 * ordered records into a single stream of in-order records.
 *
 */

#ifndef FH_SORTER_H_INCLUDED
#define FH_SORTER_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// Sentinel value indicating end of stream. Once
// all channels are terminated, sorter will emit
// a record with this value
#define EOS_MARKER UINT64_MAX

// the maximum value that the sorter can accept
#define MAX_ORDINAL_VAL  (EOS_MARKER - 1) 

// defines the output interface for sorted records
typedef bool (*sorted_fn)(void *ctx, uint64_t val, void *data, uint32_t len);
typedef struct {
    void *ctx;
    sorted_fn callback;
} fh_sort_sink_t;

// create a sorter
fh_sorter_t *
fh_sorter_new(fh_sort_sink_t output);

// destroy a sorter
void
fh_sorter_destroy(fh_sorter_t **self_p);

// register a source with an input channel. All channels
// must be registered with a unique source id before consuming
// any data records.
void
fh_sorter_register(fh_sorter_t *self, uint64_t source_id);

// start the sorter
// must be called before pushing any records to the sorter
// all input nodes must be registered before calling start.
void
fh_sorter_start(fh_sorter_t *self);

// Accept a record into the sorter
// Note: ordinal is typically a timestamp but may be any quantity that defines the
//       ordering of the records.
bool
fh_sorter_consume(fh_sorter_t *self, uint64_t ordinal, uint64_t source_id, void *data, uint32_t len);

// terminate input from a stream
bool
fh_sorter_eos(fh_sorter_t *self, uint64_t source_id);

// print some diagnostic info about the state of the sort tree
void
fh_sorter_print_info(fh_sorter_t *self, FILE *out);

// self test
void
fh_sorter_test(bool verbose);

#ifdef __cplusplus
}
#endif

#endif
