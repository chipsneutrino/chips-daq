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
fh_util_hexdump(FILE *fout, const char *desc, const void *addr, int len);

// generate a hex dump of arbitrary memory to a C string in a buffer
int
fh_util_hexdump_b(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len);


// self test
void
fh_util_test (bool verbose);

#ifdef __cplusplus
}
#endif

#endif
