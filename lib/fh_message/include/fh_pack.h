/**
 * fh_pack.h
 *
 * Implements pack/unpack functions.
 */

#ifndef FH_MESSAGE_PACK_H_INCLUDED
#define FH_MESSAGE_PACK_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// Pack data into a buffer using a python pack-like
// format string and a variable argument list
//
// All data formatted to network endianess
//
//
// +---------------------------+
// |       Format Symbols      |
// +---------------------------+
// |     |     type    | size  |
// +-----+-------------+-------+
// |  x  |  1-byte pad |   1   |
// +-----+-------------+-------+
// |  b  |  int8_t     |   1   |
// +-----+-------------+-------+
// |  B  |  uint8_t    |   1   |
// +-----+-------------+-------+
// |  h  |  int16_t    |   2   |
// +-----+-------------+-------+
// |  H  |  uint16_t   |   2   |
// +-----+-------------+-------+
// |  i  |  int32_t    |   4   |
// +-----+-------------+-------+
// |  I  |  uint32_t   |   4   |
// +-----+-------------+-------+
// |  l  |  int64_t    |   8   |
// +-----+-------------+-------+
// |  L  |  uint64_t   |   8   |
// +-----+-------------+-------+
// |  s  |  string     | <var> |
// +-----+-------------+-------+
//
//
bool
fh_pack(uint8_t *dst, size_t dst_len, size_t offset, size_t *used, const char *fmt, ...);

// variable argument list form of fh_pack
bool
fh_vpack(uint8_t *dst, size_t dst_len, size_t offset, size_t *used, const char *fmt, va_list argp);

// Unpack data from a buffer using a python unpack-like
// format string and a variable argument list
//
// +---------------------------+
// |       Format Symbols      |
// +---------------------------+
// |     |     type    | size  |
// +-----+-------------+-------+
// |  x  |  1-byte pad |   1   |
// +-----+-------------+-------+
// |  b  |  int8_t     |   1   |
// +-----+-------------+-------+
// |  B  |  uint8_t    |   1   |
// +-----+-------------+-------+
// |  h  |  int16_t    |   2   |
// +-----+-------------+-------+
// |  H  |  uint16_t   |   2   |
// +-----+-------------+-------+
// |  i  |  int32_t    |   4   |
// +-----+-------------+-------+
// |  I  |  uint32_t   |   4   |
// +-----+-------------+-------+
// |  l  |  int64_t    |   8   |
// +-----+-------------+-------+
// |  L  |  uint64_t   |   8   |
// +-----+-------------+-------+
// |  s  |  string     | <var> |
// +-----+-------------+-------+
//
// 's' requires 2 parameters, a char* and a size_t max length.
bool
fh_unpack(const uint8_t *src, size_t src_len, size_t offset, size_t *used, const char *fmt, ...);

// variable argument list form of fh_message_unpack
bool
fh_vunpack(const uint8_t *src, size_t src_len, size_t offset, size_t *used, const char *fmt, va_list argp);

// Pack data into a buffer as a null-terminated ascii string using a python pack-like
// format string and a variable argument list.
//
// Format is a null terminated ascii string with each parameter seperated by a space
// character.
//
// see fh_pack()
//
// Does not support the pad format specifier 'x'.
bool
fh_pack_ascii(uint8_t *dst, size_t dst_len, size_t offset, size_t *used, const char *fmt, ...);

// variable argument list form of fh_pack_ascii
bool
fh_vpack_ascii(uint8_t *dst, size_t dst_len, size_t offset, size_t *used, const char *fmt, va_list argp);

// use a format string to convert ascii-encoded data to binary 
bool
fh_pack_convert_ascii2bin(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used,
                          const char *fmt, char *err_buf, size_t err_buf_len);

// use a format string to convert binary-encoded data to ascii 
bool
fh_pack_convert_bin2ascii(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used,
                          const char *fmt, char *err_buf, size_t err_buf_len);

#ifdef __cplusplus
}
#endif

#endif
