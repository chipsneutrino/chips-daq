/**
 * fh_protocol.h
 *
 * Encapsulates encoding/decodeing messages to/from a serialized format.
 *
 */

#ifndef FH_PROTOCOL_H_INCLUDED
#define FH_PROTOCOL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// protocol determines how messages are encoded for transfer
typedef int (*encode_fn)(void *ctx, fh_message_t *msg, fh_stream_t *dst);
typedef int (*decode_fn)(void *ctx, fh_message_t *msg, fh_stream_t *src);
typedef struct {
   encode_fn encode;
   decode_fn decode;
   destroy_fn destroy_ctx; // destructor for context, may be NULL
} fh_protocol_impl;

// construct a protocol
fh_protocol_t *
fh_protocol_new(void *context, fh_protocol_impl impl);

//  Destroy a protocol
void
fh_protocol_destroy(fh_protocol_t **self_p);

//  create a new plain protocol
fh_protocol_t *
fh_protocol_new_plain();

//  decorate a transport with a debugging trace
//  Note: client owns delegate memory.
fh_protocol_t *
fh_protocol_new_trace(FILE *fout, fh_protocol_t *delegate);

// send an encoded message to the destination stream
int
fh_protocol_encode(fh_protocol_t *self, fh_message_t *msg, fh_stream_t *dest);

// read an encode message from the source stream
int
fh_protocol_decode(fh_protocol_t *self, fh_message_t *msg, fh_stream_t *src);

#ifdef __cplusplus
}
#endif

#endif
