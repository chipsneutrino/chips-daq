/**
 * fh_error.c
 *
 * Defines error related functionality.
 */

#include "fh_classes.h"



// Success response to all messages not requesting a value or data
// returns message of same type with zero payload
int msg_ok(fh_message_t *msg, fh_transport_t *transport) {
        fh_message_setDataLen(msg, 0);
    return fh_transport_send(transport, msg);        
}

// Invalid command
int err_invalid_cmd(fh_message_t *msg, fh_transport_t *transport    ) {
        fh_message_init(msg, ERR_SERVICE, ERR_BAD_CMD);
    return fh_transport_send(transport, msg);
}

// Wrong number of arguments
int err_bad_arg_cnt(fh_message_t *msg, fh_transport_t *transport) {
        fh_message_init(msg, ERR_SERVICE, ERR_BAD_ARG_CNT);
    return fh_transport_send(transport, msg);
}

// Argument is outside of valid range for that specific message
int err_out_of_range(fh_message_t *msg, fh_transport_t *transport) {
        fh_message_init(msg, ERR_SERVICE, ERR_OUT_OF_RANGE);
    return fh_transport_send(transport, msg);
        
}

// The given command failed due to an internal error
int err_cmd_failed(fh_message_t *msg, fh_transport_t *transport) {
        fh_message_init(msg, ERR_SERVICE, ERR_CMD_FAILED);
    return fh_transport_send(transport, msg);
}

// The system was busy and couldn't complete the command
int err_busy(fh_message_t *msg, fh_transport_t *transport) {
        fh_message_init(msg, ERR_SERVICE, ERR_BUSY);
    return fh_transport_send(transport, msg);
}

// Send a subsystem-specific message
int err_subsystem_specific(fh_message_t *msg, fh_transport_t *transport, const char* err_msg) {
    fh_message_init_full(msg, ERR_SERVICE, ERR_SUBSYSTEM_SPECIFIC, (const uint8_t*)err_msg, (strlen(err_msg) + 1) );
    return fh_transport_send(transport, msg);
}
