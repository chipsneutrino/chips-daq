/**
 * slow_ctrl_service_api.h
 *
 * Defines the client/server interface for the slow control service
 *
 **/

#ifndef _SLOW_CTRL_SERVICE_API_H_
#define _SLOW_CTRL_SERVICE_API_H_

//####################################################################
// SLOW_CTRL_SERVICE
//####################################################################

/* Set specified DAC channel (1 or 2) to specified value (0-4095)*/
#define SCS_SET_DAC 1

/* Set the specified auxiliary DAC channel to specified 12-bit value */
#define SCS_SET_AUXDAC 2

/* Set ADC timer (0-7) to delay value (min value 1) */
#define SCS_SET_ADC_TIMER_DELAY 3

/* Set cpu trigger width in units of clock cycles
 * Minimum value is 1 */
#define SCS_SET_CPUTRIG_WIDTH 4

/* Set up PWM with specified frequency and 50% duty cycle
 * but not turning on yet, unless on already */
#define SCS_SET_PWM_FREQ 5

/* Set up PWM with specified frequency and specified width
 * (units of Hz and system clock cycles, respectively)
 * but not turning on yet, unless on already */
#define SCS_SET_PWM_FREQ_WIDTH 6

/* Enable PWM output */
#define SCS_START_PWM 7

/* Disable PWM output */
#define SCS_STOP_PWM 8

/* Get temperature(s) */
#define SCS_GET_MONITOR 9

/* Get unique device ID */
#define SCS_GET_UID 10

/* Set the discriminator one-pulse mode enable (0=disabled, 1=enabled) */
#define SCS_SET_DISC_OPM_ENABLE 11

/* Set delay before frame write */
#define SCS_SET_FRAME_WRITE_DELAY 12

//####################################################################
// Debug register, etc. dumping
//####################################################################

/* print out the debug array, optional reset */
#define SCS_GET_DEBUG 13
/* start conversion on monitor ADC and get value */
#define SCS_GET_ADCMON 14
/* get the current second timestamp */
#define SCS_GET_PPS 15
/* get the current time */
#define SCS_GET_TIME 16

#endif
