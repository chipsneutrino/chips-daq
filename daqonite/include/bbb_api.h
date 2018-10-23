/**
 * BBB API - fh_library service message API for the beaglebone connections
 * 
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef BBB_API_H_
#define BBB_API_H_

#define DEFAULT_PORT 6060
#define DEFAULT_SERVER_IP "127.0.0.1"
#define MAX_MESSAGE_SIZE 4096

/// enum for the different protocols availiable
enum protocol_option { PLAIN, COBS_FRAME_V1, COBS_FRAME_V2 };

/**
 * Functions are grouped into services which have a unique type code assigned here.
 */
#define MSG_SERVICE 1

/**
 * Message Service contains functions related to the client/server messaging subsystem
 */
#define MS_STATUS 1      // query status message
#define MS_ECHO 2        // echo data
#define MS_CLOSE 3       // close the message channel

#endif
