/**
 * udaq_api.h
 *
 * Groups definitions for the udaq API communications subsystem. 
 *
 **/

#ifndef _UDAQ_API_H_
#define _UDAQ_API_H_

//####################################################################
// SERVICES
//####################################################################
// Functions are grouped into services which must have a unique type
// code assigned here.  Type "0" is reserved for errors.
#define ERR_SERVICE 0
#define MSG_SERVICE 1
#define SLOW_CTRL_SERVICE 2
#define DATA_ACCESS_SERVICE 3
#define EXP_CTRL_SERVICE 4

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

//####################################################################
// SLOW_CTRL_SERVICE
//####################################################################
// Slow Control Service function subcodes
#include "slow_ctrl_service_api.h"

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
