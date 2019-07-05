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
fh_msg_service_new(uint8_t typecode);

//  Destroy the msg_service
void
fh_msg_service_destroy(fh_msg_service_t **self_p);

// Register the service with a distpatcher
void
fh_msg_service_register(fh_msg_service_t *self, fh_dispatch_t *dispatcher);


#ifdef __cplusplus
}
#endif

#endif
