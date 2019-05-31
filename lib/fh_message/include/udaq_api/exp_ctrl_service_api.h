/**
 * exp_ctrl_service_api.h
 *
 * Defines the client/server interface for the experimental control service
 *
 **/

#ifndef _EXP_CTRL_SERVICE_API_H_
#define _EXP_CTRL_SERVICE_API_H_

//####################################################################
// EXP_CTRL_SERVICE
//####################################################################

/* Set the status for discriminator triggers: 0=disabled, 1=enabled */
#define ECS_SET_DISCRIM_ENABLE 1

/* Set the status for cpu triggers: 0=disabled, 1=enabled  */
#define ECS_SET_CPUTRIG_ENABLE 2

/* Set the control bit for 10MHz capture during cpu triggers: 0=disabled, 1=enabled  */
/* This should only be disabled for debugging purposes, unless very briefly like if it
 * only causes one capture to be skipped then probably ok.
 * N.B. for normal operation, we need 10MHz to be captured frequently, that
 * determines the phase relative to uP clock; due to counter rollover details
 * this can be skipped for one or two seconds without corrupting the timestamps
 * but then it needs to resume.
 */
#define ECS_SET_CPUTRIG_10MHZ_ENABLE 3

/* Set the livetime status for discriminator triggers: 0=deadtime, 1=livetime */
#define ECS_SET_LIVETIME_ENABLE 4

//####################################################################
// Run start / stop
//####################################################################

/* Enable data-taking */
/* RUN reset max_triggers max_seconds */
/* Depending on value of "reset", resumes previous run or start new one.
 *    reset = 0/1, 1 means reset counts & histograms to zero
 *    max_triggers = maximum triggers before automatic stop, 0 = infinite
 *    max_seconds = maximum seconds before automatic stop, 0 = infinite  
 */
#define ECS_START_RUN 5

/* stop recording data (allowing for subsequent "resume")  */
#define ECS_STOP_RUN 6

/* Get info summarizing run */
#define ECS_GET_RUN_STATS 7

//####################################################################
// Histogram acquisition setup
//####################################################################

/* Reset histogram (0-2) and enable it to record ADC channel (0-15) or -1 for none */
#define ECS_SET_HIST_ADC 8

/* Reset histogram i1 (0-2) and enable it to record ADC channel i2 (0-15) or -1 for none, with
 * filtering on trigger type (0=none, 1=discriminator, 2=cpu triggers, 3=anything) */
#define ECS_SET_HIST_ADC_MASKED 9

//####################################################################
// Recording modes / levels
//####################################################################

/* Set timestamp recording mode, i.e. what info to record in hit buffer
 * N.B. only allowed while triggers are disabled, otherwise will return error
 * 0 = do not record info in hit buffer
 * 1 = coarse timestamp only (use only a single timer input)
 * 2 = standard timestamp only
 * 3 = standard timestamp plus ToT
 * 4 = standard timestamp, ToT, dynamic selection of ADCs
 * 5 = standard timestamp, ToT, plus all CCR stamps for diagnostics
 */
#define ECS_SET_TIMESTAMP_MODE 10

/* Enable or disable filling of ADC histogram(s) */
#define ECS_SET_ADC_HIST_ENABLE 11

/* Set monitoring level
 * Specify 0=none, 1=monitoring, 2=monitor+control */
#define ECS_SET_MON_LVL 12

//####################################################################
// ADC readout configuration
//####################################################################

/* Reset threshold configuration for all ADC channels and disable */
#define ECS_RESET_ADC_THRESHOLDS 13

/* Set enable flag for specific ADC channel */
#define ECS_SET_ADC_ENABLE 14

/* Set recording threshold for specific ADC channel */
#define ECS_SET_ADC_RECORDING_THRESHOLD 15

/* Set fallback threshold for a specific ADC channel */
#define ECS_SET_ADC_FALLBACK_THRESHOLD 16

//####################################################################
// Scheduler
//####################################################################

/* Schedule a change in livetime status for discriminator triggers */
#define ECS_SCHEDULE_LIVETIME_ENABLE 17

 /* Delete all scheduled events */
#define ECS_RESET_SCHEDULE 18

#endif
