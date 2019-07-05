/**
 * frame_protocol.h
 *
 * Factory for frame protocols.
 *
 */

#ifndef FH_FRAME_PROTOCOL_H_INCLUDED
#define FH_FRAME_PROTOCOL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// support multiple implementations for
// evaluation
typedef enum {
    FP_LEGACY,    // original frame protocol with bus id field
    FP_VERSION_1, // 1 message per frame, no resend, bit errors cause fault
    FP_VERSION_2, // N frames per message,  reliable transport implementing a 1-bit
                  // sliding window (stop and wait) ARQ protocol
} fh_frame_protocol_ver;

//  create a new cobs frame protocol
fh_protocol_t *
fh_frame_protocol_new(size_t max_msg_size, fh_frame_protocol_ver version);

#ifdef __cplusplus
}
#endif

#endif
