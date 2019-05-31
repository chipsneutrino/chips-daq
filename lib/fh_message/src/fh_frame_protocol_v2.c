/**
 * fh_frame_protocol_v2.c
 *
 * Implements a modified 1-bit sliding window (stop and wait) ARQ protocol.
 * The protocol was modified such that it executes synchronously with application
 * calls to send() and receive() messages. As such this implementation does not
 * support arbitrary bidirectional bit streams, and instead relies on a strict
 * request/response use case to function correctly.
 *
 *
 * Additional Reference:
 *
 * “Sec 3.4 Sliding Window Protocols.” Computer Networks, Fifth Edition,
 *      by Andrew S. Tanenbaum and David Wetherall, Prentice Hall, 2010.
 *
 *
 * 
 * Design Details:
 *
 *   -- Message size decoupled from frame size, large messages segmented into
 *      multiple frames.
 *   -- Frames transmitted as COBS encoded bytes delimited by zero value bytes.
 *   -- Frames verified against corruption using a fletcher-16 checksum
 *   -- Positive acknowledgment sent for each frame (Stop and Wait)
 *   -- Frames retransmitted on ACK timeout or NAK
 *
 * Performance:
 *
 *   Optimal performance may require tuning the maximum frame size and timeout
 *   parameters. 
 */

#include "fh_classes.h"
#include <sys/time.h>


 // set to 1 to enable verbose debugging printout
#define DEBUG_OUTPUT 0
#define debug_print(fmt, ...) \
            do { if (DEBUG_OUTPUT) fprintf(stdout, fmt, __VA_ARGS__); } while (0)


#define FRAME_DELIMITER '\0'
#define RESEND_TIMEOUT 50


// holds a single protocol frame
typedef struct {
    size_t MAX_FRAME_SIZE; // maximum number of bytes in a frame
    size_t MAX_MSG_CHUNK;  // maximum number of decoded msg bytes that can fit in a frame
    uint8_t *enc_buf;      // storage for encoded frame data
    uint8_t *dec_buf;      // storage for decoded frame data
    int enc_len;           // length of encoded data, negative values indicate a fault
    size_t dec_len;        // length of decoded data
} frame_t;

// todo enable diagnostic counters
// holds diagnostic counters
typedef struct {
    uint32_t num_msg_sent;      // number of messages sent
    uint32_t num_msg_recv;      // number of messages received
    // uint32_t num_frame_sent;    // number of frames sent
    // uint32_t num_frame_recv;    // number of frames received
    // uint32_t num_frame_resent;  // number of frames resent
    // uint32_t num_frame_corrupt; // number of frames resent
    // uint32_t total_bytes_sent;  // number of bytes sent
    // uint32_t total_bytes_recv;  // number of bytes received
    // uint32_t num_ack_sent;      // number of ACK frames sent
    // uint32_t num_ack_received;  // number of ack frames recieved
} diag_cnt_t;

// protocol determines how messages are encoded for transfer
typedef struct {
    size_t MAX_FRAME_SIZE; // maximum number of bytes in a frame
    size_t MAX_MSG_CHUNK;  // maximum number of decoded msg bytes that can fit in a frame
    uint8_t next_seq_out;  // sequence number of the next outbound frame
    uint8_t next_seq_in;   // expected sequence number of the next inbound frame
    frame_t *data_frame;   // storage for the current/latest data frame
    frame_t *ctrl_frame;   // storage for control frames (ack)
    diag_cnt_t diag;       // diagnostic counters
    uint8_t last_ack;      // the last sequence acked

} cobs_frame_context_t;

// frame format:
//   decoded: <flag><data><checksum>
//
//   encoded: <delimiter><cobs-encoded-data><delimiter>
//
// flag bits
// ---------------------------------
// | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
// ---------------------------------
//   |   |   |   |   |   |   |   |
//   |   |   |   |   `---`---`---`---> <reserved>
//   |   |   |   |
//   |   |   `---`-------------------> MSG_CTRL:  01=MSG_END
//   |   |                                        10=MSG_START
//   |   |
//   |    `--------------------------> ACK_SEQ_NUM:   0,1
//    `------------------------------> FRAME_SEQ_NUM: 0,1
//
// NOTE: Flag bitspace is shared between the link protocol and
//       the message segmentation/reconstruction scheme.
// 
#define EXTRACT_SEQ_NUM(x) ( ((x) & 0x80) >> 7 )
#define EXTRACT_ACK_NUM(x) ( ((x) & 0x40) >> 6 )

#define ENCODE_SEQ_NUM(f,s)  ((f) |= (s) << 7 )
#define ENCODE_ACK_NUM(f,s)  ((f) |= (s) << 6 )

#define MSG_START 0x20
#define MSG_END 0x10


// get_frame() return codes 
enum frame_recv_status {frame_ok, frame_chksum_err, frame_timeout, frame_fault};


// forwards
static size_t _fh_frame_protocol_overhead(size_t message_size);
static cobs_frame_context_t * _fh_cobs_frame_context_new(size_t max_msg_size);
static void _cobs_frame_context_destroy(cobs_frame_context_t **self_p);
static void _destroy_adapter(void **self_p);
static frame_t * _fh_cobs_frame_new(size_t msg_size);
static void _fh_cobs_frame_destroy(frame_t **self_p);
static int _encode_message(void *ctx, fh_message_t *msg, fh_stream_t *dst);
static int _decode_message(void *ctx, fh_message_t *msg, fh_stream_t *src);


static enum frame_recv_status _get_frame(frame_t *frame, fh_stream_t *src, uint32_t timeout);
static bool _send_data_frame(cobs_frame_context_t *self, uint8_t flag, const uint8_t *data, size_t len, fh_stream_t *dst);
static int _send_ack_frame(cobs_frame_context_t *self, fh_stream_t *dst, bool no_repeat);
static void _frame_read(frame_t *frame, fh_stream_t *src, uint32_t timeout);
static void _frame_encode(frame_t *frame, uint8_t flag, const uint8_t *data, size_t len);
static void _frame_decode(frame_t *frame);
static bool _frame_verify(frame_t *frame);


static fh_protocol_impl COBS_FRAME_PROTOCOL_IMPL = {.encode = &_encode_message,
                                                    .decode = &_decode_message,
                                                    .destroy_ctx = &_destroy_adapter};

//  create a new cobs frame protocol v2
fh_protocol_t *
fh_frame_protocol_v2_new(size_t max_msg_size)
{
    return fh_protocol_new(_fh_cobs_frame_context_new(max_msg_size), COBS_FRAME_PROTOCOL_IMPL);
}

// calculate the worst-case extra space required to encode a message of a particular size into a frame
static size_t
_fh_frame_protocol_overhead(size_t message_size)
{
    // [<del>] + [COBS(<flags><msg><ckhsum>)] + [<del>]
    return 1 + fh_cobs_overhead(message_size + 3) + 1;
}

//  create a new frame protocol
static cobs_frame_context_t *
_fh_cobs_frame_context_new(size_t max_msg_size)
{
    cobs_frame_context_t *self = (cobs_frame_context_t *)calloc(1, sizeof(cobs_frame_context_t));
    assert(self);

    self->MAX_MSG_CHUNK = max_msg_size;
    self->MAX_FRAME_SIZE = _fh_frame_protocol_overhead(self->MAX_MSG_CHUNK);
   
   self->data_frame = _fh_cobs_frame_new(self->MAX_MSG_CHUNK);
   self->ctrl_frame = _fh_cobs_frame_new(self->MAX_MSG_CHUNK);

   self->next_seq_out = 0x0;
   self->next_seq_in = 0x0;

    return self;
}

//  Destroy a cobs frame
static void
_cobs_frame_context_destroy(cobs_frame_context_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        cobs_frame_context_t *self = *self_p;
        _fh_cobs_frame_destroy(&(self->data_frame));
        _fh_cobs_frame_destroy(&(self->ctrl_frame));
        free(self);
        *self_p = NULL;
    }
}

// adapter for the destructor
static void
_destroy_adapter(void **self_p)
{
    _cobs_frame_context_destroy((cobs_frame_context_t **)self_p);
}


//  create a new cobs frame
static frame_t *
_fh_cobs_frame_new(size_t msg_size)
{
    frame_t *self = (frame_t *)calloc(1, sizeof(frame_t));
    assert(self);

    self->MAX_FRAME_SIZE = _fh_frame_protocol_overhead(msg_size);
    self->enc_buf = calloc(1, self->MAX_FRAME_SIZE);

    self->MAX_MSG_CHUNK = msg_size + 3;     // room for <1-byte flag> + <msg> + <2-byte checksum>
    self->dec_buf = calloc(1, self->MAX_MSG_CHUNK); // room for flags + msg + checksum
 
    return self;
}

//  Destroy a cobs frame
static void
_fh_cobs_frame_destroy(frame_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        frame_t *self = *self_p;
        free(self->enc_buf);
        free(self->dec_buf);
        free(self);
        *self_p = NULL;
    }
}

// send a (protocol encoded) message on the stream
static int
_encode_message(void *ctx, fh_message_t *msg, fh_stream_t *dst)
{

    assert(ctx);
    cobs_frame_context_t *self = (cobs_frame_context_t *)ctx;


    fh_message_serialized msg_data;
    fh_message_raw(msg, &msg_data);
    assert(msg_data.length >= 0);

    uint32_t idx = 0;
    uint32_t remaining = msg_data.length;
    uint8_t flag = MSG_START;
    while(remaining > 0)
    {
        
        if(idx == 0 )
        {
            flag |= MSG_START;
        }
        size_t len = (remaining < self->MAX_MSG_CHUNK) ? remaining : self->MAX_MSG_CHUNK;
        if(len == remaining)
        {
            flag |= MSG_END;
        }

       if( !_send_data_frame(self, flag, &(msg_data.rawdata[idx]), len, dst) )
       {
        return -1;
       }

       idx += len;
       remaining -= len;
       flag = 0;
    }

    self->diag.num_msg_sent++;

    return 0;
}

// read a (protocol encoded) message from the stream
static int
_decode_message(void *ctx, fh_message_t *msg, fh_stream_t *src)
{
    assert(ctx);
    cobs_frame_context_t *self = (cobs_frame_context_t *)ctx;

    // TODO this will not stand, modify fh_message to support incremental
    //      message deserialization
    uint8_t msg_buf[65536]; // max message size
    size_t idx = 0;

    bool eom = false;
    while (!eom) {
        // read a frame and decode
        enum frame_recv_status stat = _get_frame(self->data_frame, src, RESEND_TIMEOUT);
        switch (stat) {
        case frame_fault:
            return -1;
        case frame_timeout:
        case frame_chksum_err: {
            // NOTE: This is an optimization. We could simply wait for the sender to timeout waiting for an ACK.
            //       Alternatively we can avoid waiting for the sender to timeout by re-issuing an ACK for the
            //       current sequence number (i.e. ACKing without incrementing the expected sequence number),
            //       but this must only be done ONCE in the case of consecutive corrupted frames or it can create
            //       an increasing cycle of duplicate frames/NAKS.
            //
            //       The problem situation arises when a single frame from the sender is split into multiple frames
            //       at the receiver by corruptions that introduce delimiter bytes mid-message. If the receiver issues
            //       multiple NAKs, the sender will become out-of-sync ... reading the duplicate NAK before the ACK and
            //       reissuing the frame unnecessarily. 
            _send_ack_frame(self, src, true);
            break;
        }
        case frame_ok: {

            uint8_t flag = self->data_frame->dec_buf[0];
            uint8_t seq = EXTRACT_SEQ_NUM(flag);

            debug_print("data frame OK, epected seq: %d, got seq: %d\n", self->next_seq_in, EXTRACT_SEQ_NUM(flag));
            if (seq != self->next_seq_in) {
                // duplicate packet, reissue last ACK and discard
                _send_ack_frame(self, src, false);
                debug_print("Dropping Duplicate frame seq: %d\n", seq);
                // continue;
            }
            else {
                // increment the expected sequence and issue an ACK
                self->next_seq_in = (1 - self->next_seq_in);
                _send_ack_frame(self, src, false);

                // accumulate frame data into message
                memcpy(&(msg_buf[idx]), (self->data_frame->dec_buf + 1), (self->data_frame->dec_len - 3)); //todo codify offsets
                idx += (self->data_frame->dec_len - 3);
                eom = (*(self->data_frame->dec_buf) & MSG_END);
            }
            break;
        }
        default:
             return -99; //miscode
        }
    }

    self->diag.num_msg_recv++;
    return fh_message_deserialize_from_buf(msg, msg_buf, idx);
}

// read the next frame from the stream
static enum frame_recv_status
_get_frame(frame_t *frame, fh_stream_t *src, uint32_t timeout)
{
    // read frame bytes from wire
    _frame_read(frame, src, timeout);

    if(frame->enc_len == 0){return frame_timeout;}
    if(frame->enc_len < 0) {return frame_fault;}

    // would require a miscode of _frame_read()
    //if(frame->enc_len < 2) {return frame_fault;} // considered a fault b/c _frame_read must read at least 2 delimiters

    // COBS decode
     _frame_decode(frame);

    // checksum verification
    bool intact = _frame_verify(frame);

    if(intact) {
        return frame_ok;
    }
    else
    {
        return frame_chksum_err;
    }

}

// Send a message data segment.
//
// NOTE: caller populates the message control bits of the flag byte. Remaining bits must not be set.
// NOTE: sends a frame and waits for acknowledgment, resending frame as necessary.
static bool
_send_data_frame(cobs_frame_context_t *self, uint8_t flag, const uint8_t *data, size_t len, fh_stream_t *dst)
{
    frame_t *frame = self->data_frame;

    flag |= self->next_seq_out << 7; // todo macro?
    flag |= self->next_seq_in << 6;  // todo macro?

    _frame_encode(frame, flag, data, len);


    bool acked = false;
    int wrote = 0;
    while (!acked) {
        if(DEBUG_OUTPUT)
        {
            uint16_t chksum = extract_uint16_b(frame->dec_buf, (frame->dec_len - 2) ); //todo macro, or dedicated field in frame
            debug_print("\n%s data frame %d, len: %d, chksum: %d,  with flag: %#x, seq: %d, ack: %d\n",
               ((wrote > 1) ? "RESENDING" : "SENDING"), self->next_seq_out, frame->enc_len, chksum, flag, EXTRACT_SEQ_NUM(flag),
               EXTRACT_ACK_NUM(flag));
        }
        
        // write data to stream
        wrote = fh_stream_write(dst, frame->enc_buf, frame->enc_len);

        if (wrote < 1) {
            return false;
        }

        // wait for ACK
        enum frame_recv_status stat = _get_frame(self->ctrl_frame, dst, RESEND_TIMEOUT);
        switch (stat) {
        case frame_fault: {
            return false;
        }
        case frame_timeout:
        case frame_chksum_err: {
            continue;
        }
        case frame_ok: {
            uint8_t flag = self->ctrl_frame->dec_buf[0];
            // uint8_t seq = EXTRACT_SEQ_NUM(flag); irrelevant w/out piggy backing
            uint8_t ack = EXTRACT_ACK_NUM(flag);

            if (ack == (1 - self->next_seq_out) ) {
                acked = true;
                debug_print("GOT ACK for seq %d\n", ack);
            }
            else {
                // Note: reciever is NAKing the frame
                debug_print("GOT NAK for seq %d\n", ack);
            }
            continue;
        }
        default:
             return -99; //miscode
        }
    }
    // success, increment the outbound sequence number
    self->next_seq_out = (1 - self->next_seq_out);

    return true;
}

// send an ack frame, setting the acknoledged sequect to the next expected incoming frame sequence.
static int
_send_ack_frame(cobs_frame_context_t *self, fh_stream_t *dst, bool no_repeat)
{

    // avoid multiple NAKS when requested (i.e. one nak per corrupted frame)
    if(no_repeat && (self->last_ack == self->next_seq_in) )
    {
        debug_print("Skipping duplicate ACK for seq: %d\n", self->last_ack);
        return 0;
    }
    else
    {
        self->last_ack = self->next_seq_in;
    }

    frame_t *frame = self->ctrl_frame;

    uint8_t flag = 0;
    flag |= self->next_seq_out << 7;   //todo consider what sequence numbers of ack packets mean
    flag |= self->next_seq_in << 6;
    _frame_encode(frame, flag, NULL, 0);
      
      debug_print("SENDING ACK flag: %#x, seq: %d, ack: %d\n", flag, EXTRACT_SEQ_NUM(flag), EXTRACT_ACK_NUM(flag));
    // write data to stream
    int wrote = fh_stream_write(dst, frame->enc_buf, frame->enc_len);

    if (wrote < 1) {
        return -1;
    }

    return wrote;
}

// read a delimited frame of data into the encoded buffer of
// a frame.
//
// Return status is encoded into frame->enc_len as follows:
//  <0 : fault reading from stream
//  0  : timeout reading from stream, or overflow of frame->enc_data buffer
//  >0 : length of frame content acquired into frame->enc_data buffer;
static void
_frame_read_base(frame_t *frame, fh_stream_t *src, uint32_t timeout)
{

    uint8_t cur_c;
    int idx = 0;

    // debugging variable
    uint32_t tossed = 0;

    // find a start delimeter
    do {
        int status = fh_stream_read(src, &cur_c, 1, timeout);
        if (status < 1) {
            if (status < 0) {
                // FAULT: IO error from stream
                frame->enc_len = status;
                return;
            }
            else {
                // timeout
                frame->enc_len = 0;
                return;
            }
        }

        if (cur_c != 0) {
            tossed++;
            // printf("Re-sync tossing %#x\n", cur_c);
        }
    } while (cur_c != 0);

    frame->enc_buf[idx++] = cur_c;

    while (idx < frame->MAX_FRAME_SIZE) {
        int status = fh_stream_read(src, &cur_c, 1, timeout);

        if (status < 1) {
            if (status < 0) {
                // FAULT: IO error from stream
                frame->enc_len = status;
                return;
            }
            else {
                // timeout
                frame->enc_len = 0;
                return;
            }
        }
        // printf("_frame_read()==%#x\n", cur_c);
        frame->enc_buf[idx++] = cur_c;

        if (cur_c == FRAME_DELIMITER) {
            if(idx == 2)
            {
                // EITHER:
                // 1) we synced on an end-of-frame-delimiter rather
                //    than a start of frame delimiter.
                // OR:
                // B) A bit error flipped the first byte after
                //    the start delimiter to a delimiter value
                //
                //   response:
                //   rewind one character and continue reading
                idx = 1;

                //todo
                // consider recording re-sync information
                //printf("XXXX Re-sync tossing %#x\n", cur_c);
                tossed++;
            }
            else
            {
                if(tossed)
                {
                    debug_print("re-sync discarded %"PRIu32" bytes\n", tossed);
                }

                frame->enc_len = idx;
                return;
            }
        }
    }

    debug_print("_frame_read() overflow after %d bytes read\n", idx);

    // todo
    // consider moving the status bit and maintaining the bytes read in enc_len.
    // these bytes are not usable as the stream is de-synced, but it might support
    // accounting/debugging for callers to have access to this information
    frame->enc_len = 0; // drop data
    return;
}


// wrap _frame_read_base() with diagnostic output
static void
_frame_read_log(frame_t *frame, fh_stream_t *src, uint32_t timeout)
{
    // debug only
    ///////
    uint32_t diff;
    struct timeval start;
    struct timeval stop;
    ///////

    debug_print("\n%s\n", "FRAME READING ...");

    gettimeofday(&start, 0);
    _frame_read_base(frame, src, timeout);
    gettimeofday(&stop, 0);

    diff = ((stop.tv_sec - start.tv_sec) * 1000) + ((stop.tv_usec - start.tv_usec) / 1000);

    debug_print("FRAME READ: length: %d, time(ms): %"PRIu32"\n", frame->enc_len, diff);
    return;
}

// read a delimited frame of data into the encoded buffer of
// a frame.
static void
_frame_read(frame_t *frame, fh_stream_t *src, uint32_t timeout)
{
    if (DEBUG_OUTPUT) {
        _frame_read_log(frame, src, timeout);
    }
    else {
        _frame_read_base(frame, src, timeout);
    }
}

// encode a data payload into a frame
static void
_frame_encode(frame_t *frame, uint8_t flag, const uint8_t *data, size_t len)
{
    //  frame->dec_buf format:  <flag><data><checksum>
    *(frame->dec_buf) = flag;              // flag
    memcpy(frame->dec_buf + 1, data, len); // frame data
                                           // todo: consider a gathering form of cobs encode
                                           // and fletcher16 to eliminate copying the message
                                           // data into the decode buffer
    uint16_t chksum = fh_util_fletcher16(frame->dec_buf, len + 1);
    encode_uint16(frame->dec_buf, chksum, 1 + len); // checksum
    frame->dec_len = len + 3;

    frame->enc_buf[0] = FRAME_DELIMITER;
    size_t enc_len = fh_cobs_encode(frame->dec_buf, (len + 3), frame->enc_buf + 1);
    frame->enc_buf[enc_len + 1] = FRAME_DELIMITER;

    frame->enc_len = enc_len + 2;
}

// decode the delimited, cobs-encoded data from the encoded buffer
// into the decoded buffer
static void
_frame_decode(frame_t *frame)
{
    assert(frame->enc_len > 1);
    frame->dec_len = fh_cobs_decode((frame->enc_buf + 1), (frame->enc_len - 2), frame->dec_buf);

  if(DEBUG_OUTPUT)
  {
    if(frame->dec_len >= 3)
    {
        uint8_t flag = frame->dec_buf[0];
        uint16_t chksum = extract_uint16_b(frame->dec_buf, (frame->dec_len - 2));
        debug_print("DECODED FRAME: len: %zu, chksum: %d, flag: %#x, seq: %d, ack %d\n", frame->dec_len, chksum, flag, EXTRACT_SEQ_NUM(flag), EXTRACT_ACK_NUM(flag));
    }
    else
    {
        debug_print("DECODED a short frame: %zu\n", frame->dec_len);
    }
  }
    
}

// verify the decoded data in the frame against the checksum
static bool
_frame_verify(frame_t *frame)
{
    if(frame->dec_len < 3)
    {
        //likely a corrupted COBS decoding such as the following single bit error which
        // causes the frame to be decoded into an empty byte sequence
        //                                   vv
        //   original frame:  00 04 f0 03 1b 02 01 01 04 01 44 11 00
        //   corrupted frame: 00 04 f0 03 1b 12 01 01 04 01 44 11 00
        //
        //   original decoded: f0 03 1b 00 01 00 00 01 44 11
        //   corrupted decoded: <null>
        //
        return false;

        //todo consider the case of frame->dec_len == 1. is this possible?
    }

    assert(frame->dec_len > 1);
    uint16_t chksum_orig = extract_uint16_b(frame->dec_buf, (frame->dec_len - 2));  //todo macros
    uint16_t chksum_ver = fh_util_fletcher16(frame->dec_buf, (frame->dec_len - 2)); //todo macros
    return chksum_orig == chksum_ver;
}
