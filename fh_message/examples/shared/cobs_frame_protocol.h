/**
 * fh_cobs_frame_protocol.h
 *
 * Implements a user-defined fh_protocol_t that encodes/decodes messages using
 * a COBS-encoded frame scheme.  
 *
 * NOTE: This implementation is for backward compatibility with the
 *       user-defined frame and cobs implementations. The fh_message
 *       library shall implement a COBS framing protocol that encapsulates
 *       the details.  
 * 
 */
#include "fh_library.h"


// create a new cobs-frame protocol
fh_protocol_t *
cobs_frame_protocol_new();

