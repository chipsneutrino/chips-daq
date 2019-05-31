/**
 * fh_frame_protocol_v1.c
 *
 * Encapsulates the COBS/FRAME message encoding protocol.
 *
 * Implements a basic COBS encoded frame protocol.
 *
 *   -- COBS encoded frame
 *   -- 1 message per frame
 *   -- checksum mismatch generates fault
 *   -- no frame acking/resend
 */
#include "fh_classes.h"

#define FRAME_DELIMITER '\0'

// context data
typedef struct  {
    size_t MAX_FRAME_SIZE; // maximum number of bytes in a frame
    size_t MAX_MSG_SIZE;   // maximum number of pre-cobs encoded msg bytes that can fit in a frame
    uint8_t *enc_buf;      // storage for the current frame data
    uint8_t *dec_buf;      // decoded buffer.
} cobs_frame_context_t;

// forwards
static size_t _fh_frame_protocol_overhead(size_t message_size);
static cobs_frame_context_t * _fh_cobs_frame_context_new(size_t max_msg_size);
static void _cobs_frame_context_destroy(cobs_frame_context_t **self_p);
static void _destroy_adapter(void **self_p);
static int _cobs_frame_encode(void *ctx, fh_message_t *msg, fh_stream_t *dst);
static int _cobs_frame_decode(void *ctx, fh_message_t *msg, fh_stream_t *src);
static uint16_t _fletcher16(uint8_t const *input, size_t bytes);
static int _frame_read(cobs_frame_context_t *self, fh_stream_t *src);

static fh_protocol_impl COBS_FRAME_PROTOCOL_IMPL = {.encode = &_cobs_frame_encode,
                                                    .decode = &_cobs_frame_decode,
                                                    .destroy_ctx = &_destroy_adapter};

//  create a new cobs frame protocol v1
fh_protocol_t *
fh_frame_protocol_v1_new(size_t max_msg_size)
{
    return fh_protocol_new(_fh_cobs_frame_context_new(max_msg_size), COBS_FRAME_PROTOCOL_IMPL);
}

// calculate the worst-case extra space required to encode a message of a particular size into a frame
static size_t
_fh_frame_protocol_overhead(size_t message_size)
{
    // [<del>] + [COBS(<msg><ckhsum>)] + [<del>]
    return 1 + fh_cobs_overhead(message_size + 2) + 1;
}

//  create a new cobs frame
static cobs_frame_context_t *
_fh_cobs_frame_context_new(size_t max_msg_size)
{
    cobs_frame_context_t *self = (cobs_frame_context_t *)calloc(1, sizeof(cobs_frame_context_t));
    assert(self);

    self->MAX_MSG_SIZE = max_msg_size;
    self->MAX_FRAME_SIZE = _fh_frame_protocol_overhead(self->MAX_MSG_SIZE);
    self->enc_buf = calloc(1, self->MAX_FRAME_SIZE);
    self->dec_buf = calloc(1, self->MAX_MSG_SIZE + 2); // room for msg + checksum

    return self;
}

//  Destroy a cobs frame
static void
_cobs_frame_context_destroy(cobs_frame_context_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        cobs_frame_context_t *self = *self_p;
        free(self->enc_buf);
        free(self->dec_buf);
        free(self);
        *self_p = NULL;
    }
}

// adaptor for the destructor
static void
_destroy_adapter(void **self_p)
{
    _cobs_frame_context_destroy((cobs_frame_context_t **)self_p);
}

static int
_cobs_frame_encode(void *ctx, fh_message_t *msg, fh_stream_t *dst)
{
    // IMPLEMENTATION SUMMARY
    // 1. checksum message data
    // 2. COBS encode checksum + data
    // 3. Send <del>[2]<del>

    assert(ctx);
    cobs_frame_context_t *self = (cobs_frame_context_t *)ctx;

    // serialize the message
    fh_message_serialized msg_data;
    fh_message_raw(msg, &msg_data);
    assert(msg_data.length <= self->MAX_MSG_SIZE);

    // checksum
    uint16_t chksum = _fletcher16(msg_data.rawdata, msg_data.length);

    // encode data + checksum
    // todo: consider a gathering form of cobs encode to eliminate this copy
    memcpy(self->dec_buf, msg_data.rawdata, msg_data.length);
    encode_uint16(self->dec_buf, chksum, msg_data.length);
    self->enc_buf[0] = FRAME_DELIMITER;
    size_t enc_len = fh_cobs_encode(self->dec_buf, (msg_data.length + 2), self->enc_buf + 1);
    self->enc_buf[enc_len + 1] = FRAME_DELIMITER;

    // write data to stream
    int wrote = fh_stream_write(dst, (uint8_t *)self->enc_buf, enc_len + 2);

    if (wrote < 1) {
        return -1;
    }

    return 0;
}

static int
_cobs_frame_decode(void *ctx, fh_message_t *msg, fh_stream_t *src)
{
    // IMPLEMENTATION SUMMARY
    // 1. read encoded frame
    // 2. decode frame
    // 3. verify checksum
    // 4. populate message data

    assert(ctx);
    cobs_frame_context_t *self = (cobs_frame_context_t *)ctx;

    // read a frame and decode
    size_t enc_len;
    size_t dec_len;
    if ((enc_len = _frame_read(self, src)) > 0) {
        dec_len = fh_cobs_decode((self->enc_buf + 1), (enc_len - 2), self->dec_buf);
    }
    else {
        // FAULT:  failed to read a frame
        return -1;
    }

    uint16_t chksum_orig = extract_uint16_b(self->dec_buf, (dec_len - 2));
    uint16_t chksum_ver = _fletcher16(self->dec_buf, (dec_len - 2));

    if (chksum_orig != chksum_ver) {
        // FAULT:  frame corruption
        return -1;
    }


    return fh_message_deserialize_from_buf(msg, self->dec_buf, (dec_len - 2));
}

// efficient 8 bit implementation as found on wikipedia
static uint16_t
_fletcher16(uint8_t const *input, size_t bytes)
{
    uint16_t sum1 = 0xff;
    uint16_t sum2 = 0xff;
    size_t tlen;

    while (bytes) {
        tlen = ((bytes >= 20) ? 20 : bytes);
        bytes -= tlen;
        do {
            sum2 += sum1 += *input++;
            tlen--;
        } while (tlen);
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }
    // second reduction step to reduce sums to 8 bits
    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);
    return (sum2 << 8) | sum1;
}

// read a delimeted frame of data
static int
_frame_read(cobs_frame_context_t *self, fh_stream_t *src)
{
    int idx = 0;
    uint8_t cur_c;
    bool frame_start = false;
    while (idx < self->MAX_FRAME_SIZE) {
        int status = fh_stream_read(src, &cur_c, 1, -1);

        // to do: consider non-blocking mode?
        if (status < 1) {
            if (status < 0) {
                // FAULT: IO error from stream
                return status;
            }
            else {
                // FAULT: unexpected end of stream
                return -1;
            }
        }
        self->enc_buf[idx++] = cur_c;

        if (!frame_start) {
            if (cur_c != FRAME_DELIMITER) {
                // FAULT: out of sync
                return -1;
            }
            else {
                frame_start = true;
            }
        }
        else {
            if (cur_c == FRAME_DELIMITER) {
                return idx;
            }
        }
    }

    // FAULT: buffer overflow
    return -1;
}
