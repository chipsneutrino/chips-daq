/**
 * fh_frame_protocol_v1.h
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

#ifndef FH_FRAME_PROTOCOL_V1_H_INCLUDED
#define FH_FRAME_PROTOCOL_V1_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  create a new cobs frame protocol v1
fh_protocol_t *
fh_frame_protocol_v1_new(size_t max_msg_size);

#ifdef __cplusplus
}
#endif

#endif
