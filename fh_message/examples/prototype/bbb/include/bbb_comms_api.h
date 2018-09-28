/**
 * bbb_comms_api.h
 *
 * Groups definitions for the BBB (field hub) API. 
 *
 **/

#ifndef _BBB_API_H_
#define _BBB_API_H_

//####################################################################
// SYSTEM
//####################################################################
#define DEFAULT_BBB_COMMS_PORT 7770

//####################################################################
// SERVICES
//####################################################################
// Functions are grouped into services which must have a unique type
// code assigned here.
#define MSG_SERVICE 1
// #define SLOW_CONTROL 2
#define DATA_ACCESS_SERVICE 3
#define FH_CTRL_SERVICE 5


//####################################################################
// MSG_SERVICE
//####################################################################
// Message Service function subcodes
#include "msg_service_api.h"

//####################################################################
// DATA_ACCESS_SERVICE
//####################################################################
// Data Access function subcodes
#include "data_access_service_api.h"

//####################################################################
// EXP_CTRL_SERVICE
//####################################################################
// FieldHub control funtion subcodes
#include "fh_ctrl_service_api.h"

#endif
