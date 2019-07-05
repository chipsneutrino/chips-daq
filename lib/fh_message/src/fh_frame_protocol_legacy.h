/**
 * fh_frame_protocol_legacy.h
 *
 * Encapsulates the COBS/FRAME message encoding protocol.
 *
 * Implements the legacy frame protocol.
 *
 * frame format:
 * <\0><cobs encoded data><\0>
 *
 * cobs decoded data:
 * <busid><ascii msg data><chksum>
 *
 */

#ifndef FH_FRAME_PROTOCOL_LEGACY_H_INCLUDED
#define FH_FRAME_PROTOCOL_LEGACY_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  create a new legacy frame protocol
fh_protocol_t *
fh_frame_protocol_legacy_new(size_t max_msg_size);

#ifdef __cplusplus
}
#endif

#endif
