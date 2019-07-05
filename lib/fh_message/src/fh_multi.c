/**
 * fh_multi.h
 *
 * Implements protocols for sending/receiving large data sets.
 *
 */

#include "fh_classes.h"


// forwards
bool _fh_multi_accumulate(void *ctx, fh_message_t *msg);

typedef struct {
    ssize_t expected; // number of messages expected
    size_t received;  // number of messages received
    uint8_t *buf;     // storage for reconstructed data
    size_t buf_len;   // max size of reconstructed data
    size_t buf_idx;   // current size of reconstructed data
    bool overflow;    // indicates sent data exceded buffer size
} accumulate_ctx;

bool
fh_multi_send_data(fh_message_t *msg, fh_transport_t *transport, uint8_t *err_code, const uint8_t *data, size_t len)
{
    // number of data messages needed
    uint16_t max_data_len = fh_message_getMaxDataLen();
    uint16_t nummsg = (len % max_data_len) ? (len/max_data_len + 1) : (len/max_data_len);

    // first message indicates total messages
    encode_uint16(fh_message_getData(msg), (nummsg + 1), 0); // 1 extra for first message.
    fh_message_setDataLen(msg, 2);
    int status = fh_transport_send(transport, msg);

    if (status != 0) {
        *err_code = status;
        return false;
    }

    size_t remaining = len;
    for (int i = 0; i < nummsg; i++) {
        size_t bytes_to_xfer = (remaining > max_data_len) ? max_data_len : remaining;

        fh_message_setData(msg, &(data[i * max_data_len]), bytes_to_xfer);
        status = fh_transport_send(transport, msg);

        if (status != 0) {
            *err_code = status;
            return false;
        }

        remaining -= bytes_to_xfer;
    }

    return true;
}

// send a message expecting a multi-part response and
// forward the reconstructed data to a callback.
bool
fh_multi_get_data(fh_message_t *msg, fh_transport_t *transport, uint8_t *err_code, fh_data_sink *dst, size_t max_size)
{
    // set up a sink to accumulate page data
    uint8_t buf[max_size];
    accumulate_ctx ctx = {.expected = 0, .received = 0, .buf = buf, .buf_len = max_size,
                          .buf_idx = 0, .overflow = false};

    fh_msg_sink sink = {.ctx = &ctx, .receive = &_fh_multi_accumulate};
    if (!fh_transport_exchange_multi(transport, msg, err_code, &sink)) {
        return false;
    }
    else {
        // transfer reconstructed page to handler
        if (ctx.overflow) {
            return false;
        }
        else {
            (*(dst->receive_data))(dst->ctx, ctx.buf, ctx.buf_idx);
            return true;
        }
    }
}

// a message sink that reconstructs a large data object from
// multiple messages
bool
_fh_multi_accumulate(void *ctx, fh_message_t *msg)
{

    accumulate_ctx *acc = (accumulate_ctx *)ctx;

    uint8_t *src = fh_message_getData(msg);
    uint16_t src_len = fh_message_dataLen(msg);

    // first message specifies number of messages needed
    if (acc->received == 0) {
        acc->expected = extract_uint16(msg, 0);
        src_len -= 2; // page count is not copied into buffer
    }

    acc->received++;

    //printf("Got message %zd of %zu\n", acc->received, acc->expected);

    // copy to buffer
    if (!(acc->overflow)) {
        if ((acc->buf_idx + src_len) <= acc->buf_len) {

            memcpy((void *)&(acc->buf[acc->buf_idx]), (const void *)src, src_len);
            acc->buf_idx += src_len;
        }
        else {
            // fprintf(stderr, "Page overflow: msg %zd of %zu\n", acc->received, acc->expected);
            acc->overflow = true;
        }
    }
    else
    {
        // we've oveflowed the reconstruction buffer, do not accept additional
        // data
    }

    bool complete = (acc->received == acc->expected);

    return complete;
}
