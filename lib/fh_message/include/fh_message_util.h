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

// encode a string into the message data
void
fh_encode_ascii(fh_message_t *self, const char *str, ...);

// encode an int8_t
void
encode_int8(uint8_t buf[], int8_t val, uint16_t pos);

// decode an int8_t from a messgae
int8_t
extract_int8(fh_message_t *msg, uint16_t pos);

// encode a uint8_t
void
encode_uint8(uint8_t buf[], uint8_t val, uint16_t pos);

// decode a uint8_t from a messgae
uint8_t
extract_uint8(fh_message_t *msg, uint16_t pos);

// encode a uint16_t in big-endian
void
encode_uint16(uint8_t buf[], uint16_t val, uint16_t pos);

// encode a uint16_t in little-endian
void
encode_uint16_le(uint8_t buf[], uint16_t val, uint16_t pos);

// decode a uint16_t encoded as big-endian from a messgae
uint16_t
extract_uint16(fh_message_t *msg, uint16_t pos);

// decode a uint16_t encoded as little-endian from a message
uint16_t
extract_uint16_le(fh_message_t *msg, uint16_t pos);

// decode a uint16_t encoded as big-endian from a buffer
uint16_t
extract_uint16_b(const uint8_t buf[], uint16_t pos);

// decode a uint16_t encoded as little-endian from a buffer
uint16_t
extract_uint16_ble(const uint8_t buf[], uint16_t pos);

// encode a uint32_t in big-endian
void
encode_uint32(uint8_t buf[], uint32_t val, uint16_t pos);

// decode a uint32_t encoded as big-endian from a buffer
uint32_t
extract_uint32(fh_message_t *msg, uint16_t pos);

// decode a uint32_t encoded as big-endian from a buffer
uint32_t
extract_uint32_b(const uint8_t buf[], uint16_t pos);

// encode a int32_t in big-endian
void
encode_int32(uint8_t buf[], int32_t val, uint16_t pos);

// decode a int32_t encoded as big-endian from a buffer
int32_t
extract_int32(fh_message_t *msg, int16_t pos);

// decode an int32_t encoded as big-endian from a buffer
int32_t
extract_int32_b(const uint8_t buf[], uint16_t pos);

// encode a uint64_t in big-endian
void
encode_uint64(uint8_t buf[], uint64_t val, uint16_t pos);

// decode a uint64_t encoded as big-endian from a message
uint64_t
extract_uint64(fh_message_t *msg, uint16_t pos);

// decode a uint64_t encoded as big-endian from a buffer
uint64_t
extract_uint64_b(const uint8_t buf[], uint16_t pos);

// encode a int64_t in big-endian
void
encode_int64(uint8_t buf[], int64_t val, uint16_t pos);

// decode a int64_t encoded as big-endian from a message
int64_t
extract_int64(fh_message_t *msg, int16_t pos);

// decode an int64_t encoded as big-endian from a buffer
int64_t
extract_int64_b(const uint8_t buf[], uint16_t pos);

// encode data into message payload using packe semantics
bool fh_msg_pack(fh_message_t *msg, const char *fmt, ...);

// initialize a message using pack semantics
bool fh_msg_pack_full(fh_message_t *msg, uint8_t mt, uint8_t mst, const char *fmt, ...);

// decode data from a message payload using unpack semantics
bool fh_msg_unpack(fh_message_t *msg, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
