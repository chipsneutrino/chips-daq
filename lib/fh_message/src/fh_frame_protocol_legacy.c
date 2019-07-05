/**
 * fh_frame_protocol_legacy.h
 *
 * Encapsulates the COBS/FRAME message encoding protocol.
 *
 * Implements the legacy frame protocol in use by python clients
 * in which data (sans message header) is embedded in a COBS encoded
 * frame-base message.
 *
 * frame format:
 * <\0><cobs encoded data><\0>
 *
 * cobs decoded data:
 * <busid><data><chksum>
 *
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

//  create a new legacy frame protocol
fh_protocol_t *
fh_frame_protocol_legacy_new(size_t max_msg_size)
{
    return fh_protocol_new(_fh_cobs_frame_context_new(max_msg_size), COBS_FRAME_PROTOCOL_IMPL);
}

// calculate the worst-case extra space required to encode a message of a particular size into a frame
static size_t
_fh_frame_protocol_overhead(size_t message_size)
{
    // [<del>] + [COBS(<busid><msg><ckhsum>)] + [<del>]
    return 1 + fh_cobs_overhead(1+ message_size + 2) + 1;
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

    // fh_message_hexdump(msg, "legacy encode", stderr);
    
    // IMPLEMENTATION SUMMARY
    // 1. extract message data
    // 3. checksum (bus_id + data)
    // 4. COBS encode (bus_id + data + checksum)
    // 5. Send <del><cobs data><del>

    assert(ctx);
    cobs_frame_context_t *self = (cobs_frame_context_t *)ctx;

    // set up the decoded frame:
    self->dec_buf[0] = 0; // NOTE: bus id is defined but unused
    int eom_idx = 1;

    uint16_t msg_len = fh_message_dataLen(msg);
    memcpy(&(self->dec_buf[eom_idx]), fh_message_getData(msg), msg_len);
    eom_idx += msg_len;

    // append checksum
    uint16_t chksum = _fletcher16(self->dec_buf, eom_idx);
    encode_uint16_le(self->dec_buf, chksum, eom_idx);
    eom_idx += 2;

    // encode frame
    self->enc_buf[0] = FRAME_DELIMITER;
    size_t enc_len = fh_cobs_encode(self->dec_buf, eom_idx, self->enc_buf + 1);
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
    // 4. initialize an fh_message_t with the data, synthesizing the headed as needed

    assert(ctx);
    cobs_frame_context_t *self = (cobs_frame_context_t *)ctx;

    // read a frame and decode
    int enc_len;
    size_t dec_len;

    enc_len = _frame_read(self, src);

    if (enc_len > 0) {
        dec_len = fh_cobs_decode((self->enc_buf + 1), (enc_len - 2), self->dec_buf);
    }
    else {
        // FAULT:  failed to read a frame
        return -1;
    }

    // NOTE: This is a deficiency in frame_read that can produce pathological frames such as "00 00"
    if(dec_len < 2)
    {
        return -2;
    }
    
    uint16_t chksum_orig = extract_uint16_ble(self->dec_buf, (dec_len - 2));

    uint16_t chksum_ver = _fletcher16(self->dec_buf, (dec_len - 2));

    if (chksum_orig != chksum_ver) {
        //fprintf(stderr, "BAD CHKSUM: orig %d, calc: %d\n", chksum_orig, chksum_ver);
        // FAULT:  frame corruption
        return -33;
    }

    // NOTE: (len -3) is from subtracting busid and checksum
    fh_message_init_full(msg, 0, 1, &(self->dec_buf[1]), (dec_len - 3));

    return 0;
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
    size_t idx = 0;
    uint8_t cur_c = 99;
    bool frame_start = false;
    while (idx < self->MAX_FRAME_SIZE) {
        // printf("read char...\n");
        int status = fh_stream_read(src, &cur_c, 1, -1);
        // printf("got char:%u, status:%d\n", cur_c, status);

        // to do: consider non-blocking mode?
        if (status < 1) {
            // printf("bad: %d\n", status);
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
