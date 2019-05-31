/**
 * data_access_service_api.h
 *
 * Defines the data access interface.
 *
 **/

#ifndef _DATA_ACCESS_SERVICE_API_H_
#define _DATA_ACCESS_SERVICE_API_H_

//####################################################################
// DATA_ACCESS Service
//####################################################################
// subcodes

//####################################################################
// Page polling
//####################################################################

#define DS_POLL_PAGE 1
#define DS_POLL_PAGE_INTERVAL 2

//####################################################################
// Legacy data access options
//####################################################################

/* Dump the hit buffer (hexadecimal ASCII) */
#define DS_GET_HITBUF_HEX 3

/* Dump the hit buffer (raw data) */
#define DS_GET_HITBUF_RAW 4 

/* Dump the time differences in hit buffer, with max count */
#define DS_GET_HIT_DELTA_T 5

/* Dump the hit times in hit buffer, with max count */
#define DS_GET_HIT_TIMES 6

/* Dump the hit time over threshold values in hit buffer, with max count */
#define DS_GET_HIT_TOT 7

/* Dump ADC values in hit buffer for specified channel, with max count */
#define DS_GET_HIT_ADC 8

/* Get the specified charge histogram */
#define DS_GET_HIST 9

#define DS_SUBSCRIBE 98       // available only on BBB
#define DS_UNSUBSCIBE 99      // available only on BBB


#endif
