/**
 * fh_multi.h
 *
 * Implements protocols for sending/receiving large data sets.
 *
 */

#ifndef FH_MULTI_H_INCLUDED
#define FH_MULTI_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


// callback for receiving a reconstructed large object
typedef void (*fh_receive_data_fn)(void *ctx, const uint8_t *data, size_t len);
typedef struct {
    void *ctx;
    fh_receive_data_fn receive_data;
} fh_data_sink;


// send large data array by breaking into multiple
// messages. 
bool
fh_multi_send_data(fh_message_t *msg, fh_transport_t *transport, uint8_t *err_code, const uint8_t *data, size_t len);


// send a message expecting a multi-part response and
// forward the reconstructed data to a callback. 
bool
fh_multi_get_data(fh_message_t *msg, fh_transport_t *transport, uint8_t *err_code, fh_data_sink *dst, size_t max_size);


#ifdef __cplusplus
}
#endif

#endif
