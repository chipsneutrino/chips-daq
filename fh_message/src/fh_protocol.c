/**
 * fh_protocol.c
 *
 */
#include "fh_classes.h"
#include <inttypes.h>

// protocol determines how messages are encoded for transfer
struct _fh_protocol_t {
    fh_protocol_impl *impl;
    void *context;
};

// support decorating protocols
typedef struct {
    fh_protocol_t *delegate;
} _fh_decorate_protocol_ctx;

// tracing context (extends _fh_decorate_protocol_ctx)
typedef struct {
    fh_protocol_t *delegate;
    FILE *fout;
    uint64_t msg_encoded;
    uint64_t msg_decoded;
    uint64_t bytes_encoded;
    uint64_t bytes_decoded;
} _fh_trace_protocol_ctx;

// forwards
int _fh_protocol_encode_plain(void *ctx, fh_message_t *msg, fh_stream_t *dst);
int _fh_protocol_decode_plain(void *ctx, fh_message_t *msg, fh_stream_t *src);

int _fh_protocol_encode_trace(void *ctx, fh_message_t *msg, fh_stream_t *dst);
int _fh_protocol_decode_trace(void *ctx, fh_message_t *msg, fh_stream_t *src);

int _fh_protocol_encode_cli(void *ctx, fh_message_t *msg, fh_stream_t *dst);
int _fh_protocol_decode_cli(void *ctx, fh_message_t *msg, fh_stream_t *src);

void _fh_protocol_destroy_ctx_trace(void **ctx_p);

fh_protocol_impl PLAIN_PROTOCOL_IMPL = {.encode = &_fh_protocol_encode_plain, 
                                        .decode = &_fh_protocol_decode_plain, 
                                        .destroy_ctx = NULL};

fh_protocol_impl TRACE_PROTOCOL_IMPL = {.encode = &_fh_protocol_encode_trace, 
                                        .decode = &_fh_protocol_decode_trace, 
                                        .destroy_ctx = &_fh_protocol_destroy_ctx_trace};

fh_protocol_impl CLI_PROTOCOL_IMPL = {.encode = &_fh_protocol_encode_cli,
                                      .decode = &_fh_protocol_decode_cli,
                                      .destroy_ctx = NULL};

// construct a protocol
fh_protocol_t *
fh_protocol_new(void *context, fh_protocol_impl impl)
{
    fh_protocol_t *self = (fh_protocol_t *)calloc(1,sizeof(fh_protocol_t));
    assert(self);
    self->context = context;

    self->impl = (fh_protocol_impl *)calloc(1, sizeof(fh_protocol_impl));
    assert(self->impl);
    *(self->impl) = impl;

    return self;
}

//  Destroy a protocol
void
fh_protocol_destroy(fh_protocol_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_protocol_t *self = *self_p;

        // implementation-defined context destructor
        destroy_fn destroy = self->impl->destroy_ctx;
        if (destroy) {
            (*destroy)(&(self->context));
        }
        free(self->impl);
        free(self);
        *self_p = NULL;
    }
}

//  create a new plain protocol
fh_protocol_t *
fh_protocol_new_plain()
{
    // NOTE: plain protocol does not use a context (NULL)
    return fh_protocol_new(NULL, PLAIN_PROTOCOL_IMPL);
}

//  decorate a transport with a debugging trace
//  Note: client owns delegate memory.
fh_protocol_t *
fh_protocol_new_trace(FILE *fout, fh_protocol_t *delegate)
{

    _fh_trace_protocol_ctx *protocol_ctx = (_fh_trace_protocol_ctx *)calloc(1,sizeof(_fh_trace_protocol_ctx));
    assert(protocol_ctx);
    protocol_ctx->delegate = delegate;
    protocol_ctx->fout = fout;
    return fh_protocol_new(protocol_ctx, TRACE_PROTOCOL_IMPL);
}

//  create a new CLI protocol
fh_protocol_t *
fh_protocol_new_cli()
{
    return fh_protocol_new(NULL, CLI_PROTOCOL_IMPL);
}

// send an encoded message to the destination stream
int
fh_protocol_encode(fh_protocol_t *self, fh_message_t *msg, fh_stream_t *dest)
{
    return (*(self->impl->encode))(self->context, msg, dest);
}

// read an encode message from the source stream
int
fh_protocol_decode(fh_protocol_t *self, fh_message_t *msg, fh_stream_t *src)
{
    return (*(self->impl->decode))(self->context, msg, src);
}

// ###########################################################
// CLI encoding
// ###########################################################
int
_fh_protocol_encode_cli(void *ctx, fh_message_t *msg, fh_stream_t *dst)
{
    char *str = (char *)fh_message_getData(msg);
    assert(str[fh_message_dataLen(msg)] == 0);
    fh_stream_write(dst, (uint8_t *)str, strlen(str)+1);
    return 0;
}

int
_fh_protocol_decode_cli(void *ctx, fh_message_t *msg, fh_stream_t *src)
{
    char buf[1024];
    int len = fh_stream_read(src, (uint8_t *)buf, 1024, 50);
    assert(buf[len] == 0);
    if(len > 0) {
        fh_message_init_ascii_msg(msg, buf);
        return 0;
    } else {
    	return -1;
    }

}

// ###########################################################
// plain encoding
// ###########################################################
int
_fh_protocol_encode_plain(void *ctx, fh_message_t *msg, fh_stream_t *dst)
{
    return fh_message_serialize(msg, dst);
}

int
_fh_protocol_decode_plain(void *ctx, fh_message_t *msg, fh_stream_t *src)
{
    return fh_message_deserialize(msg, src);
}

// ###########################################################
// trace encoding
// ###########################################################
int
_fh_protocol_encode_trace(void *ctx, fh_message_t *msg, fh_stream_t *dst)
{
    assert(ctx);
    _fh_trace_protocol_ctx *narrow_ctx = ((_fh_trace_protocol_ctx *)ctx);
    fh_protocol_t *delegate = narrow_ctx->delegate;

    int status = (*(delegate->impl->encode))(delegate->context, msg, dst);
    if (status == 0) {
        int encoded_bytes = fh_message_get_serialized_size(msg);
        narrow_ctx->msg_encoded++;
        narrow_ctx->bytes_encoded += encoded_bytes;
        char desc[256];
        snprintf(desc, 256, "sent msg[%d %d], size[%d], total_sent_msg[%"PRId64"], total_sent_bytes[%"PRId64"]",
                 fh_message_getType(msg), fh_message_getSubtype(msg), encoded_bytes, (narrow_ctx->msg_encoded),
                 (narrow_ctx->bytes_encoded));

        fh_message_hexdump(msg, desc, narrow_ctx->fout);
    }
    else {
        printf("send error(%d)\n", status);
    }
    return status;
}

int
_fh_protocol_decode_trace(void *ctx, fh_message_t *msg, fh_stream_t *src)
{
    assert(ctx);
    _fh_trace_protocol_ctx *narrow_ctx = ((_fh_trace_protocol_ctx *)ctx);
    fh_protocol_t *delegate = narrow_ctx->delegate;

    int status = (*(delegate->impl->decode))(delegate->context, msg, src);
    if (status == 0) {
        int decoded_bytes = fh_message_get_serialized_size(msg);
        narrow_ctx->msg_decoded++;
        narrow_ctx->bytes_decoded += decoded_bytes;
        char desc[256];
        snprintf(desc, 256, "recv msg[%d %d], size[%d], total_recv_msg[%"PRId64"], total_recv_bytes[%"PRId64"]",
                 fh_message_getType(msg), fh_message_getSubtype(msg), decoded_bytes, (narrow_ctx->msg_decoded),
                 (narrow_ctx->bytes_decoded));

        fh_message_hexdump(msg, desc, narrow_ctx->fout);
    }
    else {
        printf("receive error(%d)\n", status);
    }
    return status;
}

void
_fh_protocol_destroy_ctx_trace(void **ctx_p)
{
    // NOTE: no dynamic memory within _fh_trace_protocol_ctx
    assert(ctx_p);
    if (*ctx_p) {
        free(*ctx_p);
        *ctx_p = 0;
    }
}
