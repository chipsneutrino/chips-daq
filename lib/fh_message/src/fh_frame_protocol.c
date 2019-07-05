/**
 * fh_frame_protocol.h
 *
 * Factory for frame protocols.
 *
 */

#include "fh_classes.h"

//  create a new cobs frame protocol
fh_protocol_t *
fh_frame_protocol_new(size_t max_msg_size, fh_frame_protocol_ver version)
{
    switch (version) {
        case FP_LEGACY:
        return fh_frame_protocol_legacy_new(max_msg_size);

    case FP_VERSION_1:
        return fh_frame_protocol_v1_new(max_msg_size);
    case FP_VERSION_2:
        return fh_frame_protocol_v2_new(max_msg_size);
    default:
        abort();
    }
}

