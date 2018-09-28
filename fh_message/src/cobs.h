/* Copyright 2011, Jacques Fortier. All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted, with or without modification.
 */
#ifndef COBS_H
#define COBS_H

#include <stdint.h>
#include <stddef.h>

// constants for sizing buffers
// 
// example: x=1024
// decoded:   msg[1024] = 1024
// encoded:   msg[1024] + (1024/254 + 1) =  1029
 #define COBS_OVERHEAD(x) ( x + ((x/254) + 1)

size_t fh_cobs_encode(const uint8_t * restrict input, size_t length, uint8_t * restrict output);
size_t fh_cobs_decode(const uint8_t * restrict input, size_t length, uint8_t * restrict output);

// calculate the worst-case extra space required to encode a data of a particular size
size_t fh_cobs_overhead(size_t decoded);

// self test
void
fh_cobs_test (bool verbose);

#endif
