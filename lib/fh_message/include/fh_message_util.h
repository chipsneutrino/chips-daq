/**
 * fh_message_util.h
 *
 * Utility methods for working with messages.
 */

#ifndef FH_MESSAGE_UTIL_H_INCLUDED
#define FH_MESSAGE_UTIL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// initialize an ASCII message
void
fh_message_init_ascii_msg(fh_message_t *self, const char *str, ...);

int
fh_message_is_ascii(fh_message_t *self);

// encode a uint16_t in big-endian
void
encode_uint16(uint8_t buf[], uint16_t val, uint16_t pos);


// decode a uint16_t encoded as big-endian from a messgae
uint16_t
extract_uint16(fh_message_t *msg, uint16_t pos);

// decode a uint16_t encoded as big-endian from a buffer
uint16_t
extract_uint16_b(uint8_t buf[], uint16_t pos);

// encode a uint32_t in big-endian
void
encode_uint32(uint8_t buf[], uint32_t val, uint16_t pos);

// decode a uint32_t encoded as big-endian from a buffer
uint32_t
extract_uint32(fh_message_t *msg, uint16_t pos);

// encode a int32_t in big-endian
void
encode_int32(uint8_t buf[], int32_t val, uint16_t pos);

// decode a int32_t encoded as big-endian from a buffer
int32_t
extract_int32(fh_message_t *msg, int16_t pos);

#ifdef __cplusplus
}
#endif

#endif
