/**
 * msg_api.h
 *
 * Groups common definitions for the message sytem. 
 *
 **/

#ifndef _FH_MESSAGE_API_H_
#define _FH_MESSAGE_API_H_

//####################################################################
// SERVICES
//####################################################################
// Service codes 0 and 1 are reserved for the messaging library
//
#define ERR_SERVICE 0
#define MSG_SERVICE 1

//####################################################################
// ERRORS
//####################################################################
// Type 0 (error) subcodes
#include "error_service_api.h"

//####################################################################
// MSG_SERVICE
//####################################################################
// Message Service function subcodes
#include "msg_service_api.h"


#endif
