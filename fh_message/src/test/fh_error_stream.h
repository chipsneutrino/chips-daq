/**
 * fh_error_stream.h
 *
 * Implements a stream decorator that injects errors into the stream.
 * Used for testing protocol implementations.
 */

#ifndef FH_ERROR_STREAM_H_INCLUDED
#define FH_ERROR_STREAM_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  Create a new error stream
fh_stream_t * fh_error_stream_new(fh_stream_t *delegate, float burst_event_rate, FILE *log);

#ifdef __cplusplus
}
#endif

#endif
