/**
 * fh_transport.c
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
 *
 */

#include "fh_classes.h"


// support dynamic tracing
typedef struct {
    bool enabled;
    void *original;
} trace_t;

// instance data
struct _fh_transport_t {
    fh_protocol_t *protocol;
    fh_stream_t *stream;
    trace_t protocol_trace;
    trace_t stream_trace;
};


//  base constructor
fh_transport_t *
fh_transport_new(fh_protocol_t *protocol, fh_stream_t *stream)
{
    fh_transport_t *self = (fh_transport_t *)calloc(1,sizeof(fh_transport_t));
    assert(self);

    self->protocol = protocol;
    self->stream = stream;

    self->protocol_trace.enabled = false;
    self->protocol_trace.original = NULL;

    self->stream_trace.enabled = false;
    self->stream_trace.original = NULL;
    
    return self;
}

//  Create a new socket transport
fh_transport_t *
fh_transport_new_socket(int socket_fd)
{
    fh_protocol_t *protocol = fh_protocol_new_plain();
    fh_stream_t *stream = fh_stream_new_socket(socket_fd);

    return fh_transport_new(protocol, stream);
}

//  Create a new file transport
fh_transport_t *
fh_transport_new_file(int fdin, int fdout)
{
    fh_protocol_t *protocol = fh_protocol_new_plain();
    fh_stream_t *stream = fh_stream_new_file_desc(fdin, fdout);

    return fh_transport_new(protocol, stream);
}

//  Create a new CLI transport
fh_transport_t *
fh_transport_new_cli(FILE *fin, FILE *fout)
{
    fh_protocol_t *protocol = fh_protocol_new_cli();
    fh_stream_t *stream = fh_stream_new_cli(fin, fout);

    return fh_transport_new(protocol, stream);
}

//  Destroy the transport
void
fh_transport_destroy(fh_transport_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_transport_t *self = *self_p;
        if (self->protocol) {
            fh_protocol_destroy(&(self->protocol));
        }
        if (self->stream) {
            fh_stream_destroy(&(self->stream));
        }

        if (self->protocol_trace.original) {
            fh_protocol_destroy((fh_protocol_t**)&(self->protocol_trace.original));
        }
        if (self->stream_trace.original) {
            fh_stream_destroy((fh_stream_t**)&(self->stream_trace.original));
        }

        free(self);
        *self_p = NULL;
    }
}

// Set the encoding protocol
void
fh_transport_set_protocol(fh_transport_t *self, fh_protocol_t *protocol)
{
    fh_protocol_destroy(&(self->protocol));
    self->protocol = protocol;
}

// Set the stream
void
fh_transport_set_stream(fh_transport_t *self, fh_stream_t *stream)
{
    fh_stream_destroy(&(self->stream));
    self->stream = stream;
}

// send an serialized message over the transport
int
fh_transport_send(fh_transport_t *self, fh_message_t *msg)
{
    return fh_protocol_encode(self->protocol, msg, self->stream);
}

// receive a serialized message over the transport
int
fh_transport_receive(fh_transport_t *self, fh_message_t *msg)
{
    return fh_protocol_decode(self->protocol, msg, self->stream);
}

// enable tracing message encoding/decoding
void
fh_transport_enable_protocol_trace(fh_transport_t *self, FILE *fout)
{
    assert(!(self->protocol_trace.enabled));

    fh_protocol_t *trace = fh_protocol_new_trace(fout, self->protocol);

    self->protocol_trace.original = self->protocol;
    self->protocol = trace;
    self->protocol_trace.enabled = true;
}

// disable tracing message encoding/decoding
void
fh_transport_disable_protocol_trace(fh_transport_t *self)
{
    if (!(self->protocol_trace.enabled)) {
        return;
    }

    fh_protocol_t *trace = self->protocol;
    self->protocol = self->protocol_trace.original;
    self->protocol_trace.enabled = false;
    self->protocol_trace.original = NULL;

    fh_protocol_destroy((fh_protocol_t**)&trace);
}

// enable tracing message reading/wirting
void
fh_transport_enable_stream_trace(fh_transport_t *self, FILE *fout)
{
    assert(!(self->stream_trace.enabled));

    fh_stream_t *trace = fh_stream_new_trace(fout, self->stream);

    self->stream_trace.original = self->stream;
    self->stream = trace;
    self->stream_trace.enabled = true;
}

// disable tracing message reading/wirting
void
fh_transport_disable_stream_trace(fh_transport_t *self)
{
    if (!(self->stream_trace.enabled)) {
        return;
    }

    fh_stream_t *trace = self->stream;
    self->stream = self->stream_trace.original;
    self->stream_trace.enabled = false;
    self->stream_trace.original = NULL;

    fh_stream_destroy((fh_stream_t**)&trace);
}


