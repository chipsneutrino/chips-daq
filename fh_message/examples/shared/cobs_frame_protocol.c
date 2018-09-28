/**
 * fh_cobs_frame_protocol.h
 *
 * Implements a user-defined fh_protocol_t that encodes/decodes messages using
 * a COBS-encoded frame scheme.  
 *
 * NOTE: This implementation is for backward compatibility with the
 *       user-defined frame and cobs implementations. The fh_message
 *       library shall implement a COBS framing protocol that encapsulates
 *       the details.  
 * 
 */
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "fh_library.h"
#include "cobs_frame_protocol.h"
#include "frame.h"

// per-instance context
typedef struct {
    Frame frame;
} cobs_frame_context_t;

// forwards
cobs_frame_context_t* _cobs_frame_context_new();
void _cobs_frame_context_destroy(cobs_frame_context_t **self_p);
void _destroy_adapter(void **self_p);
int _cobs_frame_encode(void *ctx, fh_message_t *msg, fh_stream_t *dst);
int _cobs_frame_decode(void *ctx, fh_message_t *msg, fh_stream_t *src);
int _frame_read(Frame *f, fh_stream_t *src);

// static impelmentation
fh_protocol_impl  COBS_FRAME_PROTOCOL = {&_cobs_frame_encode, &_cobs_frame_decode, &_destroy_adapter};

// create a new cobs-frame protocol
fh_protocol_t *
cobs_frame_protocol_new()
{
    return fh_protocol_new(_cobs_frame_context_new(), COBS_FRAME_PROTOCOL);
}

cobs_frame_context_t*
_cobs_frame_context_new()
{
    cobs_frame_context_t *self = (cobs_frame_context_t *)calloc(1,sizeof(cobs_frame_context_t));
    assert(self);

    return self;
}

//  Destroy the context
void
_cobs_frame_context_destroy(cobs_frame_context_t **self_p)
{
     assert(self_p);
    if (*self_p) {
        cobs_frame_context_t *self = *self_p;
        free(self);
        *self_p = NULL;
    }
}

// adaptor for the destructor
void
_destroy_adapter(void **self_p)
{
    _cobs_frame_context_destroy((cobs_frame_context_t **)self_p);
}

int
_cobs_frame_encode(void *ctx, fh_message_t *msg, fh_stream_t *dst)
{
    assert(ctx);
    cobs_frame_context_t *self = (cobs_frame_context_t*)ctx;

    // serialize the message into the frame
    fh_message_serialized msg_data;
    fh_message_raw(msg, &msg_data);
    frameinit(&(self->frame), (char*)(msg_data.rawdata), msg_data.length );

    // encode frame
    encode(&(self->frame));

    // todo consider refactoring frame.h/readwrite.h to integrate the stream write.
    int wrote = fh_stream_write(dst, (uint8_t*) self->frame.dat, self->frame.datLen);
    if(wrote < 1)
    {
        return -1;
    }

    return 0;
}


int
_cobs_frame_decode(void *ctx, fh_message_t *msg, fh_stream_t *src)
{
    assert(ctx);
    cobs_frame_context_t *self = (cobs_frame_context_t*)ctx;

    // read a frame and decode COBS section
    if( _frame_read(&(self->frame), src) != 0)
    {
        return -1;
    }

    return fh_message_deserialize_from_buf(msg, (uint8_t*)(self->frame.msg), self->frame.msgLen);
}



// adapted from readwrite.c
int 
_frame_read(Frame *f, fh_stream_t *src){
    uint8_t buf[MAXDATA] = {0};
    int check = 0;
    int t = 0; // buffer idx
    uint8_t c;
    int done = 0;
    static uint8_t lastc = FRAME_DELIMITER;
    static int inFrame = 0;
    while ((!done) && (t < MAXDATA)) {
        // Grab one character
        // check = (*read)(ctx, &c, 1);
        check = fh_stream_read(src, &c, 1, -1);
            // serial? non-blocking?
            // if (check < 0) {
            // printf("%s\n",strerror(errno));
            // break;
            // } else if (check == 0)
            // continue;

            //tcp
            if (check < 1) {
                if(check < 0)
                {
                   printf("%s\n",strerror(errno));
                return check;
                }
                else
                {
                    //unexpected end of stream
                    return -1;
                }
                break;
            }

            if (c == FRAME_DELIMITER) {
            done = inFrame;
            inFrame = (lastc == FRAME_DELIMITER);
            }

        //DEBUG
        //printf("read: %d (c = 0x%02x)\tinFrame %d\tdone %d\n", check, c, inFrame, done);
            if (inFrame || done)
                buf[t++] = c;

            // Keep track of previous character in case we
            // get de-synced
            lastc = c;
        }

    // We bailed on the read but didn't finish
//printf("DONE is %d AFTER WHILE LOOP\n",done);
//printf("t is %d\n",t);
    if (!done) return -3;
    frameinit_encoded(f, buf, t);
    return decode(f);
}
