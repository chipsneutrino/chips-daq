/**
 * server_api.h
 *
 * Groups definitions for the server API. 
 *
 **/

#ifndef _SERVER_API_H_
#define _SERVER_API_H_

//####################################################################
// SYSTEM
//####################################################################
#define DEFAULT_PORT 6060


//####################################################################
// SERVICES
//####################################################################
// Functions are grouped into services which have a unique type code
// assigned here.
#define MSG_SERVICE 1
#define CALC_SERVICE 99


//####################################################################
// MSG_SERVICE
//####################################################################
// Message Service contains functions related to the client/server
// messaging subsystem
//
#define MS_STATUS 1      // query status message
#define MS_ECHO 2        // echo data
#define MS_CLOSE 3       // close the message channel

//####################################################################
// ECHO_SERVICE
//####################################################################
// A trivial service providing calculation functions
#include "calc_api.h"

#endif
