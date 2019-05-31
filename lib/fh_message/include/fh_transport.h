/**
 * fh_transport.h
 *
 * Encapsulates reading/writing serialized messages.  The interface is
 * broken into two layers that may be varied independently. The protocol layer
 * encapsulates how a message is encode for transport.  The steam layer transfers
 * the encoded bytes to/from the source/destination channel.
 *
 *            fh_transport_send (*msg)
 *            fh_transport_receive (*msg)
 *                +
 *                |
 *                |
 *       +--------v---------+
 *       |                  | fh_transport_send: send message.
 *       |                  |
 *       |    transport     | fh_transport_receive: receive message.
 *       |                  |
 *       |                  |
 *       +--------+---------+
 *                |
 *                |
 *            fh_protocol_encode (*msg, *stream)
 *            fh_protocol_decode (*msg, *stream)
 *                |
 *       +--------v---------+
 *       |                  |   fh_protocol_encode: Encode message content and
 *       |                  |                       write to stream.
 *       |     protocol     |
 *       |                  |   fh_protocol_decode: read encoded message from 
 *       |                  |                       stream and decode.
 *       +------------------+
 *                |
 *                |
 *       fh_stream_write (*uint8_t[], len)
 *       fh_stream_read (*uint8_t[], len)
 *                |
 *       +--------v---------+
 *       |                  |  fh_stream_write: write encoded bytes to destination.
 *       |                  |
 *       |      stream      |  fh_stream_read:  read encoded bytes from source.
 *       |                  |
 *       |                  |
 *       +------------------+
 *
 */

#ifndef FH_TRANSPORT_H_INCLUDED
#define FH_TRANSPORT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  Create a new socket transport
fh_transport_t *
fh_transport_new_socket(int socket_fh);

//  Create a new file transport
fh_transport_t *
fh_transport_new_file(int fdin, int fdout);

//  Create a new CLI transport
fh_transport_t *
fh_transport_new_cli(FILE *fin, FILE *fout);

//  base constructor
fh_transport_t *
fh_transport_new(fh_protocol_t *protocol, fh_stream_t *stream);

//  Destroy the transport
void
fh_transport_destroy(fh_transport_t **self_p);

// Set the encoding protocol
void
fh_transport_set_protocol(fh_transport_t *self, fh_protocol_t *protocol);

// Set the stream
void
fh_transport_set_stream(fh_transport_t *self, fh_stream_t *stream);

// send a serialized message over the transport
int
fh_transport_send(fh_transport_t *self, fh_message_t *msg);

// receive a serialized message over the transport
int
fh_transport_receive(fh_transport_t *self, fh_message_t *msg);

// enable tracing message encoding/decoding
void
fh_transport_enable_protocol_trace(fh_transport_t *self, FILE *fout);

// disable tracing message encoding/decoding
void
fh_transport_disable_protocol_trace(fh_transport_t *self);

// enable tracing message reading/wirting
void
fh_transport_enable_stream_trace(fh_transport_t *self, FILE *fout);
// disable tracing message reading/wirting
void
fh_transport_disable_stream_trace(fh_transport_t *self);

#ifdef __cplusplus
}
#endif

#endif
