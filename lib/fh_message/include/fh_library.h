/**
 * fh_library.h
 *
 * Public API for field hub messaging library.
 */

#ifndef FH_LIBRARY_H_INCLUDED
#define FH_LIBRARY_H_INCLUDED

// external dependencies
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


//  Opaque class references
typedef struct _fh_message_t fh_message_t;
typedef struct _fh_dispatch_t fh_dispatch_t;
typedef struct _fh_transport_t fh_transport_t;
typedef struct _fh_service_t fh_service_t;
typedef struct _fh_msg_service_t fh_msg_service_t;
typedef struct _fh_slow_ctrl_service_t fh_slow_ctrl_service_t;
typedef struct _fh_sorter_t fh_sorter_t;
typedef struct _fh_data_access_service_t fh_data_access_service_t;
typedef struct _fh_exp_ctrl_service_t fh_exp_ctrl_service_t;
typedef struct _fh_stream_t fh_stream_t;
typedef struct _fh_protocol_t fh_protocol_t;
typedef struct _fh_connector_t fh_connector_t;


// function signatures
typedef void (*destroy_fn)(void **ctx_pn); // generic destructor

// Public API
#include "api/msg_api.h"

//  Public classes
#include "fh_connector.h"
#include "fh_data_access_service.h"
#include "fh_dispatch.h"
#include "fh_error.h"
#include "fh_exp_ctrl_service.h"
#include "fh_frame_protocol.h"
#include "fh_message.h"
#include "fh_message_util.h"
#include "fh_msg_service.h"
#include "fh_msg_translator.h"
#include "fh_multi.h"
#include "fh_pack.h"
#include "fh_protocol.h"
#include "fh_service.h"
#include "fh_slow_ctrl_service.h"
#include "fh_sorter.h"
#include "fh_stream.h"
#include "fh_transport.h"
#include "fh_util.h"
#include "fh_ascii_service.h"

// Public test functions
#include "fh_test_functions.h"



#endif
