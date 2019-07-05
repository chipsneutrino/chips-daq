/**
 * fh_error.h
 *
 * Defines error related functionality.
 */

#ifndef FH_ERROR_H_INCLUDED
#define FH_ERROR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

int msg_ok(fh_message_t *msg, fh_transport_t *transport);
int err_bad_arg_cnt(fh_message_t *msg, fh_transport_t *transport);
int err_invalid_cmd(fh_message_t *msg, fh_transport_t *transport);
int err_out_of_range(fh_message_t *msg, fh_transport_t *transport);
int err_cmd_failed(fh_message_t *msg, fh_transport_t *transport);
int err_busy(fh_message_t *msg, fh_transport_t *transport);
int err_subsystem_specific(fh_message_t *msg, fh_transport_t *transport, const char* err_msg);

#ifdef __cplusplus
}
#endif

#endif
