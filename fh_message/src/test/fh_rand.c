/**
 * fh_rand.h
 *
 * Implements a pseudo-random generator.
 */
#include "../fh_classes.h"

// ##################################################################
// # fh_rand
// ##################################################################

struct _fh_rand_t {
    uint32_t rand_seed;
};

fh_rand_t *
fh_rand_new(uint32_t rand_seed)
{
    fh_rand_t *self = (fh_rand_t *)calloc(1, sizeof(fh_rand_t));
    assert(self);
    self->rand_seed = rand_seed;
    return self;
}

void
fh_rand_destroy(fh_rand_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_rand_t *self = *self_p;
        free(self);
        *self_p = NULL;
    }
}

uint32_t
fh_rand(fh_rand_t *self)
{
    // self->rand_seed = (8253729 * self->rand_seed + 2396403);
    // return self->rand_seed % 32767;

    // uint32_t lfsr = self->rand_seed;
    // self->rand_seed = (lfsr >> 1) ^ (-(lfsr & (uint32_t)1) & (((uint32_t)1 << 30) | ((uint32_t)1 << 27)));
    // return (~(self->rand_seed) );

    self->rand_seed = ((uint64_t)self->rand_seed * 279470273u) % 0xfffffffb;
    return self->rand_seed;
}

uint32_t
fh_rand_int(fh_rand_t *self, uint32_t min, uint32_t max)
{
    return fh_rand(self) % (max - min + 1) + min;
}

float
fh_rand_float(fh_rand_t *self, float min, float max)
{
    float scale = fh_rand(self) / (float)0xffffffff; /* [0, 1.0] */
    return min + scale * (max - min);                /* [min, max] */
}

// ##################################################################
// # fh_rand_ord_seq
// ##################################################################

struct _fh_rand_ord_seq_t {
  fh_rand_t *rand;
  uint32_t max_stride;
  uint64_t last_val;
};

fh_rand_ord_seq_t *
fh_rand_ord_seq_new(uint32_t rand_seed, uint64_t start_val, uint32_t max_stride)
{
    fh_rand_ord_seq_t *self = (fh_rand_ord_seq_t *)calloc(1, sizeof(fh_rand_ord_seq_t));
    assert(self);

    self->last_val = start_val;
    self->max_stride = max_stride;

    self->rand = fh_rand_new(rand_seed);

    return self;
}

void
fh_rand_ord_seq_destroy(fh_rand_ord_seq_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_rand_ord_seq_t *self = *self_p;
        fh_rand_destroy(&(self->rand));
        free(self);
        *self_p = NULL;
    }
}

uint64_t
rand_ord_seq_next(fh_rand_ord_seq_t *self)
{
    uint32_t inc = fh_rand_int(self->rand, 0, self->max_stride);
    uint64_t next_val = self->last_val + inc;
    if (next_val < self->last_val) {
        next_val = UINT64_MAX;
    }
    self->last_val = next_val;

    return self->last_val;
}
