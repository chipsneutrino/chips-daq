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
    FILE *dst;
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
    self->stream_trace.dst = NULL;

    self->stream_trace.enabled = false;
    self->stream_trace.dst = NULL;
    
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
    // maintain tracing if already enabled
    bool was_tracing = self->protocol_trace.enabled;
    FILE *dst = self->protocol_trace.dst;

    // clean up the old tracing properly
    if (self->protocol_trace.enabled) {
        fh_transport_disable_protocol_trace(self);
    }

    fh_protocol_destroy(&(self->protocol));
    self->protocol = protocol;

    if (was_tracing) {
        fh_transport_enable_protocol_trace(self, dst);
    }
}

// Set the stream
void
fh_transport_set_stream(fh_transport_t *self, fh_stream_t *stream)
{
    // maintain tracing if already enabled
    bool was_tracing = self->stream_trace.enabled;
    FILE *dst = self->stream_trace.dst;

    // clean up the old tracing properly
    if(self->stream_trace.enabled)
    {
       fh_transport_disable_stream_trace(self);
    }

    fh_stream_destroy(&(self->stream));

    self->stream = stream;

    if(was_tracing)
    {
        fh_transport_enable_stream_trace(self, dst);
    }
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

// send a message and receive a response
// returns false and populates err_code if error encountered
bool
fh_transport_exchange(fh_transport_t *self, fh_message_t *msg, uint8_t *err_code)
{

    uint8_t mt_sent = fh_message_getType(msg);
    uint8_t mst_sent = fh_message_getSubtype(msg);

    // Send the message
    if(fh_transport_send(self, msg) !=0 ) {
        *err_code = ERR_COMMS;
        return false;
    }

    // Receieve reply message
    if(fh_transport_receive(self, msg)!=0) {
        *err_code = ERR_COMMS;
        return false;
    }

    // Check for error message
    uint8_t mt_rcv = fh_message_getType(msg);
    uint8_t mst_rcv = fh_message_getSubtype(msg);
    if (mt_rcv != mt_sent || mst_rcv != mst_sent) {

        if (mt_rcv == ERR_SERVICE) {
            // server responded with an error
            *err_code = mst_rcv;
        }
        else {
            // violation of API
            *err_code = ERR_UNSPECIFIED;
        }

        return false;
    }

    return true;
}

// send a message and receive multiple response messages
// returns false and populates err_code if error encountered
//
// note: caller is responsible for metering the number of expected messages
//       via the return value of sink->recieve.
bool
fh_transport_exchange_multi(fh_transport_t *self, fh_message_t *msg, uint8_t *err_code, fh_msg_sink *sink)
{

    uint8_t mt_sent = fh_message_getType(msg);
    uint8_t mst_sent = fh_message_getSubtype(msg);

    // Send the message
    if(fh_transport_send(self, msg) !=0 ) {
        *err_code = ERR_COMMS;
        return false;
    }

    // Receieve reply message(s) until client
    // indicates exchange is complete (or error)
    bool complete = false;
    while (!complete) {
        if (fh_transport_receive(self, msg) != 0) {
            *err_code = ERR_COMMS;
            return false;
        }

        // Check for error message
        uint8_t mt_rcv = fh_message_getType(msg);
        uint8_t mst_rcv = fh_message_getSubtype(msg);
        if (mt_rcv != mt_sent || mst_rcv != mst_sent) {

            if (mt_rcv == ERR_SERVICE) {
                // server responded with an error
                *err_code = mst_rcv;
            }
            else {
                // violation of API
                *err_code = ERR_UNSPECIFIED;
            }

            return false;
        }

        // pass message to callback
        complete = (*(sink->receive))(sink->ctx, msg);

    }
    return true;
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
    self->protocol_trace.dst = fout;
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
    self->protocol_trace.dst = NULL;

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
    self->stream_trace.dst = fout;
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
    self->stream_trace.dst = NULL;

    fh_stream_destroy((fh_stream_t**)&trace);
}


