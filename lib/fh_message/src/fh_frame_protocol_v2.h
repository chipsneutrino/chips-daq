/**
 * fh_frame_protocol_v2.h
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

#ifndef FH_FRAME_PROTOCOL_V2_H_INCLUDED
#define FH_FRAME_PROTOCOL_V2_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  create a new cobs frame protocol v2
fh_protocol_t *
fh_frame_protocol_v2_new(size_t max_msg_size);

#ifdef __cplusplus
}
#endif

#endif
