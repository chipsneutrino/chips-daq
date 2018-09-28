/**
 * udaq_api.h
 *
 * Groups definitions for the udaq API communications subsystem. 
 *
 **/

#ifndef _UDAQ_API_H_
#define _UDAQ_API_H_

//####################################################################
// COMMS CONSTANTS
//####################################################################
#define DEFAULT_UDAQ_COMMS_PORT 6060


//####################################################################
// SERVICES
//####################################################################
// Functions are grouped into services which must have a unique type
// code assigned here.
#define MSG_SERVICE 1
// #define SLOW_CONTROL 2
#define DATA_ACCESS_SERVICE 3
#define EXP_CTRL_SERVICE 4


//####################################################################
// MSG_SERVICE
//####################################################################
// Message Service function subcodes
#include "msg_service_api.h"

//####################################################################
// DATA_ACCESS_SERVICE
//####################################################################
// Data Access funtion subcodes
#include "data_access_service_api.h"

//####################################################################
// EXP_CTRL_SERVICE
//####################################################################
// Experimental Control funtion subcodes
#include "exp_ctrl_service_api.h"

#endif
