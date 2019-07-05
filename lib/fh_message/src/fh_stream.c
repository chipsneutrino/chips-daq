/**
 * fh_stream.c
 */
#include "fh_classes.h"

// bare metal arm toolchain does not provide poll()
#ifdef DISABLE_POLLING
   #include <sys/select.h>
#else
   #include <sys/poll.h>
#endif

#include <sys/time.h>

// stream determines how encoded messages are transfered
struct _fh_stream_t {
    fh_stream_impl *impl;
    void *context;
};


// stream implementation contexts
#define BUF_LEN 4096
typedef struct {
    int fdin;                 // inbound file descriptor
    int fdout;                // outbound file descriptor
    uint8_t rd_buf[BUF_LEN];  // read buffer
    uint8_t *rd_buf_tail;     // index of first buffered  byte
    size_t rd_buf_len;        // length of buffered data
} _fh_file_desc_ctx;

typedef struct {
    FILE * fin;                 // inbound file pointer
    FILE * fout;                // outbound file pointer
} _fh_file_ctx;

typedef struct {
    uint8_t *buf;
    size_t length;
    size_t index;
} _fh_buf_stream_ctx;

// support decorating streams and protocols
typedef struct {
    fh_stream_t *delegate;
} _fh_decorate_stream_ctx;

// tracing contexts (extends _fh_decorate_stream_ctx)
typedef struct {
    fh_stream_t *delegate;
    FILE *fout;
    uint64_t bytes_in;
    uint64_t bytes_out;
} _fh_trace_stream_ctx;

// forwards
int _fh_transport_read_socket(void *ctx, uint8_t *buf, size_t length, int timeout);
int _fh_transport_write_socket(void *ctx, uint8_t *buf, size_t length);

int _fh_transport_read_file_desc(void *ctx, uint8_t *buf, size_t length, int timeout);
int _fh_transport_write_file_desc(void *ctx, uint8_t *buf, size_t length);

int _fh_transport_read_buffer(void *ctx, uint8_t *buf, size_t length, int timeout);
int _fh_transport_write_buffer(void *ctx, uint8_t *buf, size_t length );

int _fh_transport_read_trace(void *ctx, uint8_t *buf, size_t length, int timeout);
int _fh_transport_write_trace(void *ctx, uint8_t *buf, size_t length);

int _fh_transport_read_decorate(void *ctx, uint8_t *buf, size_t length, int timeout);
int _fh_transport_write_decorate(void *ctx, uint8_t *buf, size_t length);

fh_stream_impl FILE_DESC_STREAM = {&_fh_transport_read_file_desc, &_fh_transport_write_file_desc, NULL};
fh_stream_impl BUFFER_STREAM = {&_fh_transport_read_buffer, &_fh_transport_write_buffer, NULL};
fh_stream_impl TRACE_STREAM = {&_fh_transport_read_trace, &_fh_transport_write_trace, NULL};
fh_stream_impl DECORATED_STREAM = {&_fh_transport_read_decorate, &_fh_transport_write_decorate, NULL};

// construct a stream
fh_stream_t *
fh_stream_new(void *context, fh_stream_impl impl)
{
    fh_stream_t *self = (fh_stream_t *)calloc(1,sizeof(fh_stream_t));
    assert(self);
    self->context = context;

    self->impl = (fh_stream_impl *)calloc(1,sizeof(fh_stream_impl));
    assert(self->impl);
    self->impl->read = impl.read;
    self->impl->write = impl.write;
    self->impl->destroy_ctx = impl.destroy_ctx;

    return self;
}

//  Destroy a stream
void
fh_stream_destroy(fh_stream_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_stream_t *self = *self_p;

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

//  Create a new socket stream
fh_stream_t *
fh_stream_new_socket(int socket_fd)
{
    return fh_stream_new_file_desc(socket_fd, socket_fd);
}

//  Create a new file stream
fh_stream_t *
fh_stream_new_file_desc(int fdin, int fdout)
{
    _fh_file_desc_ctx *stream_ctx = (_fh_file_desc_ctx *)calloc(1, sizeof(_fh_file_desc_ctx));
    assert(stream_ctx);

    stream_ctx->fdin = fdin;
    stream_ctx->fdout = fdout;

    stream_ctx->rd_buf_tail = stream_ctx->rd_buf;
    stream_ctx->rd_buf_len = 0;

    // int status = fcntl(fileno(fin), F_SETFL, O_NONBLOCK);
    // printf("set fd %d to nonblock. status: %d\n", fileno(fin), status);

    return fh_stream_new(stream_ctx, FILE_DESC_STREAM);
}

//  Create a new buffer-backed stream
fh_stream_t *
fh_stream_new_buffer(uint8_t *buf, size_t length)
{
    _fh_buf_stream_ctx *stream_ctx = (_fh_buf_stream_ctx *)calloc(1,sizeof(_fh_buf_stream_ctx));
    assert(stream_ctx);
    stream_ctx->buf = buf;
    stream_ctx->length = length;
    stream_ctx->index = 0;

    return fh_stream_new(stream_ctx, BUFFER_STREAM);
}

//  decorate a stream with a debugging trace
//  Note: client owns delegate memory.
fh_stream_t *
fh_stream_new_trace(FILE *fout, fh_stream_t *delegate)
{

    _fh_trace_stream_ctx *stream_ctx = (_fh_trace_stream_ctx *)calloc(1,sizeof(_fh_trace_stream_ctx));
    assert(stream_ctx);
    stream_ctx->delegate = delegate;
    stream_ctx->fout = fout;

    return fh_stream_new(stream_ctx, TRACE_STREAM);
}

int
fh_stream_read(fh_stream_t *self, uint8_t *buf, size_t length, int timeout)
{
    return (*(self->impl->read))(self->context, buf, length, timeout);
}

int
fh_stream_write(fh_stream_t *self, uint8_t *buf, size_t length)
{
    //fh_util_hexdump(stdout, "# stream write #", buf, length);
    return (*(self->impl->write))(self->context, buf, length);
}


// ###########################################################
// file desc stream impl
// ###########################################################
// forward
int _fh_transport_fill_buffer(_fh_file_desc_ctx *self, int timeout);

int
_fh_transport_read_file_desc(void *ctx, uint8_t *buf, size_t length, int timeout)
{
    assert(ctx);

    _fh_file_desc_ctx *self = (_fh_file_desc_ctx *)ctx;

    // fill from buffer if available
    size_t read_cnt = 0;
    if (self->rd_buf_len > 0) {
        size_t from_buf = length > self->rd_buf_len ? self->rd_buf_len : length;

        memcpy(buf, self->rd_buf_tail, from_buf);

        self->rd_buf_tail += from_buf;
        self->rd_buf_len -= from_buf;

            length -= from_buf;
        buf += from_buf;

        read_cnt += from_buf;
    }

    if (length == 0) {
        return read_cnt;
    }
    else {
        //  More data is required, re-fill buffer
        int status = _fh_transport_fill_buffer(self, timeout);
        if (status < 1) {
            if (status < 0) {
                return status;
            }
            else {
                return read_cnt; // timout with possible non-empty read from buffer
            }
        }

        if (self->rd_buf_len > 0) {
            size_t from_buf = length > self->rd_buf_len ? self->rd_buf_len : length;
            memcpy(buf, self->rd_buf, from_buf);

            self->rd_buf_tail += from_buf;
            self->rd_buf_len -= from_buf;
            read_cnt += from_buf;

            return read_cnt;
        }
        else {
            return read_cnt;
        }
    }
}

int
_fh_transport_write_file_desc(void *ctx, uint8_t *buf, size_t length)
{
    assert(ctx);

    // int wrote =  fwrite(buf, 1, length, ((_fh_file_desc_ctx *)ctx)->fout);
    // fflush(((_fh_file_desc_ctx *)ctx)->fout);
    // return wrote;

    return write(((_fh_file_desc_ctx *)ctx)->fdout, buf, length);
}

// read available data from the stream into the read buffer.
int
_fh_transport_fill_buffer(_fh_file_desc_ctx *self, int timeout)
{
#ifndef DISABLE_POLLING
    struct pollfd pf;
    pf.fd = self->fdin;
    pf.events = POLLIN;

    int status = poll(&pf, 1, timeout);  
    if (status < 1) {
        return status;
    }

    if (pf.revents & POLLIN) {

        int stat = read(self->fdin, self->rd_buf, BUF_LEN);
        self->rd_buf_tail = self->rd_buf;
        self->rd_buf_len = stat;
        return stat;
    }
    else {
        // should never get hear if poll is well behaved
        fprintf(stderr, "Poll with \'POLLIN\' returned without POLLIN set\n");
        return -3;
    }
#else
    return 0;
#endif
}

// ###########################################################
// buffer stream impl
// ###########################################################
int
_fh_transport_read_buffer(void *ctx, uint8_t *buf, size_t length, int timeout)
{
    _fh_buf_stream_ctx *narrow = (_fh_buf_stream_ctx *)ctx;
    size_t available = narrow->length - narrow->index;
    if (length <= available) {
        memcpy(buf, narrow->buf + narrow->index, length);
        narrow->index += length;
        return length;
    }
    else {
        memcpy(buf, narrow->buf + narrow->index, available);
        narrow->index += available;
        return available;
    }
}

int
_fh_transport_write_buffer(void *ctx, uint8_t *buf, size_t length )
{
    //todo
    return -3;
}


// ###########################################################
// trace stream impl
// ###########################################################
int
_fh_transport_read_trace(void *ctx, uint8_t *buf, size_t length, int timeout)
{
    int read = _fh_transport_read_decorate(ctx, buf, length, timeout);

    assert(ctx);
    _fh_trace_stream_ctx *narrow_ctx = ((_fh_trace_stream_ctx *)ctx);

    // trace
    if (read > 0) {
        narrow_ctx->bytes_in += read;
        char desc[256];
        snprintf(desc, 256, "stream:read[%d] of [%llu]", read, (narrow_ctx->bytes_in) );
        fh_util_hexdump(narrow_ctx->fout, desc, buf, read);
    }
    else {
        printf("read error(%d)\n", read);
    }

    return read;
}

int
_fh_transport_write_trace(void *ctx, uint8_t *buf, size_t length)
{
    int wrote = _fh_transport_write_decorate(ctx, buf, length);

    assert(ctx);
    _fh_trace_stream_ctx *narrow_ctx = ((_fh_trace_stream_ctx *)ctx);

    // trace
    if (wrote > 0) {
        narrow_ctx->bytes_out += wrote;
        char desc[256];
        snprintf(desc, 256, "stream:wrote[%d] of [%llu]", wrote, (narrow_ctx->bytes_out) );
        fh_util_hexdump(narrow_ctx->fout, desc, buf, wrote);
    }
    else {
        printf("write error(%d)\n", wrote);
    }

    return wrote;
}

// ###########################################################
// decorate stream impl
// ###########################################################
int
_fh_transport_read_decorate(void *ctx, uint8_t *buf, size_t length, int timeout)
{
    assert(ctx);
    _fh_decorate_stream_ctx *narrow_ctx = ((_fh_decorate_stream_ctx *)ctx);
    fh_stream_t *delegate = narrow_ctx->delegate;
    int read = (*(delegate->impl->read))(delegate->context, buf, length, timeout);

    return read;
}

int
_fh_transport_write_decorate(void *ctx, uint8_t *buf, size_t length)
{
    assert(ctx);
    _fh_decorate_stream_ctx *narrow_ctx = ((_fh_decorate_stream_ctx *)ctx);
    fh_stream_t *delegate = narrow_ctx->delegate;
    int wrote = (*(delegate->impl->write))(delegate->context, buf, length);

    return wrote;
}
