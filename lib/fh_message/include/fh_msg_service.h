/**
 * fh_message_service.h
 *
 */

#ifndef FH_MSG_SERVICE_H_INCLUDED
#define FH_MSG_SERVICE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  Create a new msg_service
fh_msg_service_t *
fh_msg_service_new(void);

//  Destroy the msg_service
void
fh_msg_service_destroy(fh_msg_service_t **self_p);

#ifdef __cplusplus
}
#endif

#endif
