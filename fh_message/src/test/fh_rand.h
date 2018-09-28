/**
 * fh_rand.h
 *
 * Implements a pseudo-random generator.
 */

#ifndef FH_RAND_H_INCLUDED
#define FH_RAND_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fh_rand_t fh_rand_t;
typedef struct _fh_rand_ord_seq_t fh_rand_ord_seq_t;

// ##################################################################
// # fh_rand
// ##################################################################

fh_rand_t *
fh_rand_new();

void
fh_rand_destroy(fh_rand_t **self_p);

 uint32_t
fh_rand(fh_rand_t *self);

 uint32_t
fh_rand_int(fh_rand_t *self, uint32_t min, uint32_t max);

 float
fh_rand_float(fh_rand_t *self, float min, float max);


// ##################################################################
// # fh_rand_ord_seq
// ##################################################################

fh_rand_ord_seq_t *
fh_rand_ord_seq_new(uint32_t rand_seed, uint64_t start_val, uint32_t max_stride);

void
fh_rand_ord_seq_destroy(fh_rand_ord_seq_t **self_p);

uint64_t
rand_ord_seq_next(fh_rand_ord_seq_t *self);


#ifdef __cplusplus
}
#endif

#endif
