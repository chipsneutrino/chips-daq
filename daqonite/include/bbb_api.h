/*
 * bbb_api.h
 * Groups definitions for the BBB API.
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#ifndef BBB_API_H_
#define BBB_API_H_

//####################################################################
// SYSTEM
//####################################################################
#define DEFAULT_PORT 6060
#define DEFAULT_SERVER_IP "127.0.0.1"
#define MAX_MESSAGE_SIZE 4096

enum protocol_option { PLAIN, COBS_FRAME_V1, COBS_FRAME_V2 };

//####################################################################
// SERVICES
//####################################################################
// Functions are grouped into services which have a unique type code
// assigned here.
#define MSG_SERVICE 1

//####################################################################
// MSG_SERVICE
//####################################################################
// Message Service contains functions related to the client/server
// messaging subsystem
//
#define MS_STATUS 1      // query status message
#define MS_ECHO 2        // echo data
#define MS_CLOSE 3       // close the message channel

#endif /* BBB_API_H_ */
