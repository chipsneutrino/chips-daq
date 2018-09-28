/**
 * fh_util.h
 *
 * Utility functions.
 *
 */

#ifndef FH_UTIL_H_INCLUDED
#define FH_UTIL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// generate a fletcher-16 checksum 
uint16_t
fh_util_fletcher16(uint8_t const *input, size_t bytes);

// generate a hex dump of arbitrary memory to a stream
void
fh_util_hexdump(FILE *fout, char *desc, void *addr, int len);


// self test
void
fh_util_test (bool verbose);

#ifdef __cplusplus
}
#endif

#endif
