/**
 * fh_stream.h
 *
 * Encapsulates reading/writing serialized data.
 *
 */

#ifndef FH_STREAM_H_INCLUDED
#define FH_STREAM_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// stream determines how encoded messages are transfered
typedef int (*read_fn)(void *ctx, uint8_t *buf, size_t length, int timeout);
typedef int (*write_fn)(void *ctx, uint8_t *buf, size_t length);
typedef struct {
    read_fn read;
    write_fn write;
    destroy_fn destroy_ctx; // destructor for context, may be NULL
} fh_stream_impl;

// construct a stream
fh_stream_t *
fh_stream_new(void *context, fh_stream_impl impl);

//  Destroy a stream
void
fh_stream_destroy(fh_stream_t **self_p);

//  Create a new socket stream
fh_stream_t *
fh_stream_new_socket(int socket_fd);


//  Create a new file stream
fh_stream_t *
fh_stream_new_file_desc(int fdin, int fdout);

//  Create a new buffer-backed stream
fh_stream_t *
fh_stream_new_buffer(uint8_t *buf, size_t length);

//  decorate a stream with a debugging trace
//  Note: client owns delegate memory.
fh_stream_t *
fh_stream_new_trace(FILE *fout, fh_stream_t *delegate);

int
fh_stream_read(fh_stream_t *self, uint8_t *buf, size_t length, int timeout);

int
fh_stream_write(fh_stream_t *self, uint8_t *buf, size_t length);

#ifdef __cplusplus
}
#endif

#endif
