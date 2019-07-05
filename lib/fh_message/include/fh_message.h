/**
 * fh_message.h
 *
 * Encapsulates the on-the-wire message format.
 */

#ifndef FH_MESSAGE_H_INCLUDED
#define FH_MESSAGE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const uint8_t *rawdata;
    uint16_t length;
} fh_message_serialized;

//  Create a new message
fh_message_t *
fh_message_new(void);

//  Destroy the message
void
fh_message_destroy(fh_message_t **self_p);

// zero-out a message resulting in a message of type 0, sutbype 0
// with no data payload.
void
fh_message_clear(fh_message_t *self);

// initialze a message with a type/subtype with no data payload
void
fh_message_init(fh_message_t *self, uint8_t type, uint8_t subtype);

// initialize a message with a type/subtype and payload data.
void
fh_message_init_full(fh_message_t *self, uint8_t type, uint8_t subtype, const uint8_t *data, uint16_t length);

// initialize a message with the contents of another message
void
fh_message_copy_from(fh_message_t *self, fh_message_t *msg);

// write the serialize message to a transport stream
int
fh_message_serialize(fh_message_t *self, fh_stream_t *dest);

// consume a message from a transport stream, initializing a message with the deserialized content
int
fh_message_deserialize(fh_message_t *self, fh_stream_t *src);

// write the serialize message to buffer
int
fh_message_serialize_to_buf(fh_message_t *self, uint8_t *buf, size_t length);

// deserialize a message from a buffer
int
fh_message_deserialize_from_buf(fh_message_t *self, const uint8_t *buf, size_t length);

// access the raw serialized message bytes
void
fh_message_raw(fh_message_t *self, fh_message_serialized *serialized);

// access the total size of a message in serialized form
uint32_t
fh_message_get_serialized_size(fh_message_t *self);

// message type
uint8_t
fh_message_getType(fh_message_t *self);

// message subtype
uint8_t
fh_message_getSubtype(fh_message_t *self);

// access the message payload data
uint8_t *
fh_message_getData(fh_message_t *self);

// message payload data length
uint16_t
fh_message_dataLen(fh_message_t *self);

// set the message type
void
fh_message_setType(fh_message_t *self, uint8_t t);

// set the message subtype
void
fh_message_setSubtype(fh_message_t *self, uint8_t st);

// set the message data payload (with copy)
void
fh_message_setData(fh_message_t *self, const uint8_t *d, uint16_t size);

// set the message data payload length
void
fh_message_setDataLen(fh_message_t *self, uint16_t l);

// generate a hex dump of the serialized message to a stream
void
fh_message_hexdump(fh_message_t *self, const char *desc, FILE *fout);

// get the maximum possible data length (bytes)
uint16_t
fh_message_getMaxDataLen(void);

#ifdef __cplusplus
}
#endif

#endif
