/**
 * fh_classes.h
 *
 * Private header file.
 *
 */

#ifndef FH_CLASSES_H_INCLUDED
#define FH_CLASSES_H_INCLUDED

// Standard includes
#include <assert.h>
// #include <ctype.h>
#include <errno.h>
#include <inttypes.h>
// #include <float.h>
#include <limits.h>
// #include <math.h>
// #include <setjmp.h>
// #include <signal.h>
// #include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <time.h>
#include <unistd.h>
#include <fcntl.h>


//  External public API
#include "../include/fh_library.h"

//  Internal private API
#include "fh_socket_util.h"
#include "cobs.h"
#include "fh_frame_protocol_v1.h"
#include "fh_frame_protocol_v2.h"
#include "test/fh_rand.h"
#include "test/fh_error_stream.h"
#include "test/fh_selftest.h"
#include "test/fh_transport_test.h"


#endif
