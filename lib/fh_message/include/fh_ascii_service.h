/**
 * ascii_service.h
 *
 * Provides a translation layer that accepts ascii formatted
 * messages which will be translated to binary format, 
 * re-dispatched
 *
 **/

#ifndef _ASCII_SERVICE_H_
#define _ASCII_SERVICE_H_

#include "fh_library.h"

typedef struct _fh_ascii_service_t fh_ascii_service_t;

// create a new ascii service
fh_ascii_service_t * fh_ascii_srv_new(uint8_t typecode, msg_dict_t *msg_dict);

//  Destroy the ascii service
void fh_ascii_srv_destroy(fh_ascii_service_t **self_p);

// register the service with a message distpatcher
void fh_ascii_srv_register_service(fh_ascii_service_t *self, fh_dispatch_t *dispatcher);

// Create a protocol instance that provides a command line interface
// legacy_framed == false: message exchange uses new-line terminated ascii strings
// legacy_framed == true: message exchange uses new-line terminated ascii strings
//                        encapsulated in legacy cobs-encoded frames.
fh_protocol_t * fh_ascii_srv_new_cli_proto(fh_ascii_service_t *self, bool legacy_framed);

// emit the "OK\n" prompt
int fh_ascii_emit_prompt(fh_ascii_service_t *self, fh_message_t *msg, fh_transport_t *transport);

#endif
