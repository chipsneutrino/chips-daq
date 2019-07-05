/**
 * error_service_api.h
 *
 * Error code subtypes of message type 0
 * This will likely move to a more general location, as these are not udaq-specific
 **/

#ifndef _ERROR_SERVICE_API_H_
#define _ERROR_SERVICE_API_H_

//####################################################################
// ERROR_SERVICE
//####################################################################
// Error Service contains error code definitions
//
#define ERR_OK 0
#define ERR_UNSPECIFIED 1
#define ERR_SUBSYSTEM_SPECIFIC 2
#define ERR_BAD_ARG_CNT 3
#define ERR_BAD_VALUE 4
#define ERR_OUT_OF_RANGE 5
#define ERR_BAD_CMD 6
#define ERR_CMD_FAILED 7
#define ERR_BUSY 8
#define ERR_COMMS 9

#endif
