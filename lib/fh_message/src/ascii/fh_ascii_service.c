/*
 * fh_ascii_service.c
 *
 * A service that supports ascii oriented client.
 *
 * Attepts to translate inbound ascii formated messages
 * from the client, dispatche to proper service and then
 * translate outbound binary response into an ascii response.
 *
 * Useful during prototyping when command-line interaction
 * is desirable.
 */

#include "../fh_classes.h"
#include "fh_ascii_api.h"
#include "fh_library.h"

struct _ascii_request {
    fh_message_t *asciimsg;    // holds the original message and translated response
    fh_transport_t *transport; // transport for outbound messages
    size_t resp_cnt;           // count of response messages
    bool err_flag;             // set if the response was an error
};
typedef struct _ascii_request ascii_request;

struct _fh_ascii_service_t {
    uint8_t typecode;
    fh_service_t *service;
    fh_dispatch_t *dispatcher;
    fh_transport_t *intercept_xport; // transport for intercepting binary reponses
    fh_message_t *binmsg;            // holds the binary translation and binary response
    msg_dict_t *msg_dict;            // holds translation rules
    fh_msg_translator_t *translator; // holds translation functions
    char *err_buf;                   // staging space for error messages
    size_t err_buf_len;
    ascii_request request; // holds transient objects scoped to a single request
};

// forwards
static int _handle_ascii_message(void *ctx, fh_message_t *msg, fh_transport_t *transport);
static void _init_request(fh_ascii_service_t *self, fh_transport_t *transport, fh_message_t *asciimsg);
static int _transmit_reponse(fh_ascii_service_t *self);
static int _complete_request(fh_ascii_service_t *self);
static int _handle_ascii_message_internal(fh_ascii_service_t *self);
static int _reencode_response(void *ctx, fh_message_t *msg, fh_stream_t *dst);
static int _fail(void *ctx, fh_message_t *msg, fh_stream_t *dst);
static void _fh_ascii_init_resp(fh_ascii_service_t *self, fh_message_t *msg, const char *str, ...);

static fh_protocol_t * _fh_protocol_new_cli(uint8_t mt);
static fh_protocol_t * _fh_protocol_new_legacy_cobs(uint8_t mt, size_t max_msg_size);


// intercepting protocol (handles outbound only)
static fh_protocol_impl INTERCEPT_PROTOCOL_IMPL = {.encode = &_reencode_response,
                                                   .decode = &_fail,
                                                   .destroy_ctx = NULL};

// create a new ascii service
fh_ascii_service_t *
fh_ascii_srv_new(uint8_t typecode, msg_dict_t *msg_dict)
{
    fh_ascii_service_t *self = (fh_ascii_service_t *)calloc(1, sizeof(fh_ascii_service_t));
    assert(self);

    self->typecode = typecode;

    fh_service_t *srvc = fh_service_new(self->typecode, self);
    fh_service_register_function(srvc, ASCII_CMD, &_handle_ascii_message);

    self->service = srvc;

    // set up a transport to intercept binary responses
    self->intercept_xport = fh_transport_new(fh_protocol_new(self, INTERCEPT_PROTOCOL_IMPL), NULL);

    self->binmsg = fh_message_new();

    self->msg_dict = msg_dict;
    self->translator = fh_msg_translator_new(self->msg_dict);

    self->err_buf_len = 64;
    self->err_buf = calloc(1, 64);

    return self;
}

//  Destroy the ascii service
void
fh_ascii_srv_destroy(fh_ascii_service_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_ascii_service_t *self = *self_p;

        fh_service_destroy(&(self->service));
        fh_transport_destroy(&(self->intercept_xport));
        fh_message_destroy(&(self->binmsg));

        fh_msg_translator_destroy(&(self->translator));

        free(self->err_buf);

        free(self);
        *self_p = NULL;
    }
}

// register ascci service with a dispatcher
void
fh_ascii_srv_register_service(fh_ascii_service_t *self, fh_dispatch_t *dispatcher)
{
    fh_dispatch_register_service(dispatcher, self->service);
    self->dispatcher = dispatcher;
}

// Create a protocol instance that provides a command line interface
// legacy_framed == false: message exchange uses new-line terminated ascii strings
// legacy_framed == true: message exchange uses new-line terminated ascii strings
//                        encapsulated in legacy cobs-encoded frames.
fh_protocol_t *
fh_ascii_srv_new_cli_proto(fh_ascii_service_t *self, bool legacy_framed)
{
    if (legacy_framed) {
        return _fh_protocol_new_legacy_cobs(self->typecode, fh_message_getMaxDataLen());
        // return _fh_protocol_new_legacy_cobs(self->typecode, 256);
    }
    else {
        return _fh_protocol_new_cli(self->typecode);
    }
}

// emit the "OK\n" prompt
int
fh_ascii_emit_prompt(fh_ascii_service_t *self, fh_message_t *msg, fh_transport_t *transport)
{
    fh_message_init_full(msg, self->typecode, ASCII_RESP, (uint8_t *)ascii_ok, ascii_ok_len);
    return fh_transport_send(transport, msg);
}

// handles incoming ascii messages
static int
_handle_ascii_message(void *ctx, fh_message_t *msg, fh_transport_t *transport)
{
    // 1. stash the request objects (transport and original message)
    // 2. translate message to binary
    // 3. re-dispatch, passing our custom transport to intercept the response
    // 4. on response intercept, translate back to ascii and send response back
    //    on original transport

    fh_ascii_service_t *self = (fh_ascii_service_t *)ctx;

    // Note: stashing reference to the original request objects. We need
    //       these to send the translated response when we intercept the
    //       binary response
    _init_request(self, transport, msg);

    int status = _handle_ascii_message_internal(self);

    if (status == 0) {
        return _complete_request(self);
    }
    else {
        return status;
    }
}

// internal method called after invariants set up in wrapper method
static int
_handle_ascii_message_internal(fh_ascii_service_t *self)
{

    self->err_buf[0] = '\0';

    bool success = fh_msg_translate_ascii2bin(self->translator, self->request.asciimsg, self->binmsg, self->err_buf,
                                              self->err_buf_len);
    if (!success) {

        size_t errlen = strnlen(self->err_buf, self->err_buf_len);
        if (errlen > 0 && errlen < self->err_buf_len) {
            _fh_ascii_init_resp(self, self->request.asciimsg, self->err_buf);
        }
        else {
            init_ascii_xlat_err(self->err_buf, self->err_buf_len, __FILE__, __LINE__);
            _fh_ascii_init_resp(self, self->request.asciimsg, self->err_buf);
        }

        return _transmit_reponse(self);
    }

    // prevent recursion
    if (fh_message_getType(self->binmsg) == fh_service_getTypecode(self->service)) {
        init_ascii_xlat_err(self->err_buf, self->err_buf_len, __FILE__, __LINE__);
        _fh_ascii_init_resp(self, self->request.asciimsg, self->err_buf);
        return _transmit_reponse(self);
    }

    // re-dispatch as binary message
    return fh_dispatch_handle(self->dispatcher, self->binmsg, self->intercept_xport);

}

// All requests begin here.
// enforces sane management of transient fields
static void
_init_request(fh_ascii_service_t *self, fh_transport_t *transport, fh_message_t *asciimsg)
{
    // should only be handling one request at a time
    assert(self->request.transport == NULL);
    assert(self->request.asciimsg == NULL);

    // hold referenses to the original request objects
    self->request.transport = transport;
    self->request.asciimsg = asciimsg;
    self->request.err_flag = false;
    self->request.resp_cnt = 0;
}

// All responses handled here.
// enforces sane management of transient fields
static int
_transmit_reponse(fh_ascii_service_t *self)
{
    assert(self->request.transport != NULL); // we should always respond using the original transport
    assert(self->request.asciimsg != NULL);  // we should always respond using the original message

    uint8_t *data = fh_message_getData(self->request.asciimsg);
    uint16_t len = fh_message_dataLen(self->request.asciimsg);

    self->request.resp_cnt++;

    if (len > 0) {
        // set error flag if first character of response is "?"
        if (self->request.resp_cnt == 1) {
            self->request.err_flag = (data[0] == '\?');
        }
        // set the message type
        fh_message_setType(self->request.asciimsg, self->typecode);

        return fh_transport_send(self->request.transport, self->request.asciimsg);
    }
    else {
        // was an empty binary response, stripped of message header in translation
        // only the trailing "OK" prompt will be sent
        return 0;
    }
}

// All responses handled here.
// enforces sane management of transient fields
static int
_complete_request(fh_ascii_service_t *self)
{
    assert(self->request.transport != NULL); // we should always respond using the original transport
    assert(self->request.asciimsg != NULL);  // we should always respond using the original message

    // A few possibilities on the client side
    //
    // 1. a successful command with no response data
    // > OK
    //
    // 2. a successful command with response data
    // > RESPONSE DATA
    // > OK
    //
    // 3. An error message
    // >?ERROR MSG
    //

    int status = 0;
    if (!self->request.err_flag) {
        status = fh_ascii_emit_prompt(self, self->request.asciimsg, self->request.transport);
    }

    // void out references to transients
    self->request.asciimsg = NULL;
    self->request.transport = NULL;
    self->request.err_flag = false;

    return status;
}

// re-encode outboud messages as ascii
static int
_reencode_response(void *ctx, fh_message_t *binmsg, fh_stream_t *dst)
{
    // 1. translate response to ascii
    // 2. send ascii message via original transport

    fh_ascii_service_t *self = (fh_ascii_service_t *)ctx;

    assert(self->binmsg == binmsg); // what we sent out should be what we got back

    self->err_buf[0] = '\0';
    bool success = fh_msg_translate_bin2ascii(self->translator, self->binmsg, self->request.asciimsg, self->err_buf,
                                              self->err_buf_len);

    if (!success) {

        size_t errlen = strnlen(self->err_buf, self->err_buf_len);
        if (errlen > 0 && errlen < self->err_buf_len) {
            _fh_ascii_init_resp(self, self->request.asciimsg, self->err_buf);
        }
        else {
            init_ascii_xlat_err(self->err_buf, self->err_buf_len, __FILE__, __LINE__);
            _fh_ascii_init_resp(self, self->request.asciimsg, self->err_buf);
        }

        return _transmit_reponse(self);
    }

    // verify subtype set by translator
    uint8_t mst = fh_message_getSubtype(self->request.asciimsg);
    if ( !(mst == ASCII_RESP || mst == BINARY_RESP) ){
        init_ascii_xlat_err(self->err_buf, self->err_buf_len, __FILE__, __LINE__);
        _fh_ascii_init_resp(self, self->request.asciimsg, self->err_buf);
    }

    return _transmit_reponse(self);
}

// bound to the inbound side of the intercept protocol
static int
_fail(void *ctx, fh_message_t *msg, fh_stream_t *dst)
{
    return -1;
}

// initialize an ASCII response message
static void
_fh_ascii_init_resp(fh_ascii_service_t *self, fh_message_t *msg, const char *str, ...)
{
    uint8_t *dst = fh_message_getData(msg);
    uint16_t dst_len = fh_message_getMaxDataLen();

    va_list arg;

    va_start(arg, str);
    int status = vsnprintf((char *)dst, dst_len, str, arg);
    va_end(arg);

    if (status >= 0) {
        fh_message_setType(msg, self->typecode);
        fh_message_setSubtype(msg, ASCII_RESP);
        // include null termination
        fh_message_setDataLen(msg, strlen((char *)dst) + 1);
    }
    else {
        // vsnprintf failed
        init_ascii_xlat_err(self->err_buf, self->err_buf_len, __FILE__, __LINE__);
        _fh_ascii_init_resp(self, self->request.asciimsg, self->err_buf);
    }
}

// ############################################################################
// a protocol implementation that converts from ascii message exchange to
// command-line interface.
//
// Message exhange is newline terminated ascii strings.
// Responses begining with '?' are considerrred errors.
// Non-error responses emit an "OK" prompt at completion.
//
// ############################################################################
typedef struct {
    uint8_t mt;        // type of generated ascii msg
} _fh_cli_protocol_ctx;

static int
_fh_protocol_encode_cli(void *ctx, fh_message_t *msg, fh_stream_t *dst);
static int
_fh_protocol_decode_cli(void *ctx, fh_message_t *msg, fh_stream_t *src);
static void
_fh_protocol_destroy_ctx_cli(void **ctx_p);

static fh_protocol_impl CLI_PROTOCOL_IMPL = {.encode = &_fh_protocol_encode_cli,
                                             .decode = &_fh_protocol_decode_cli,
                                             .destroy_ctx = _fh_protocol_destroy_ctx_cli};

//  create a new CLI protocol
static fh_protocol_t *
_fh_protocol_new_cli(uint8_t mt)
{
    _fh_cli_protocol_ctx *ctx = (_fh_cli_protocol_ctx *)calloc(1, sizeof(_fh_cli_protocol_ctx));
    assert(ctx);
    ctx->mt = mt;
    return fh_protocol_new(ctx, CLI_PROTOCOL_IMPL);
}

static int
_fh_protocol_encode_cli(void *ctx, fh_message_t *msg, fh_stream_t *dst)
{

    _fh_cli_protocol_ctx *self = (_fh_cli_protocol_ctx *)ctx;

    uint8_t *data = fh_message_getData(msg);
    uint16_t len = fh_message_dataLen(msg);

    // must be an ascii message type
    assert(fh_message_getType(msg) == self->mt);

    // must be null terminated payload
    assert(data[len - 1] == 0);

    // replace null termination with newline
    if (len > 0 && (data[len - 1] == '\0')) {
        data[len - 1] = '\n';
    }

    fh_stream_write(dst, data, len);

    return 0;
}

static int
_fh_protocol_decode_cli(void *ctx, fh_message_t *msg, fh_stream_t *src)
{
    _fh_cli_protocol_ctx *self = (_fh_cli_protocol_ctx *)ctx;

    fh_message_init(msg, self->mt, ASCII_CMD);
    uint8_t *payload = fh_message_getData(msg);
    uint16_t max = fh_message_getMaxDataLen();

    int idx = 0;
    while (idx < max) {

        int status = fh_stream_read(src, &(payload[idx]), 1, -1);

        if (status < 1) {
            if (status == 0) {
                // stream violated timeouut contract
                return -3;
            }
            else {
                return status;
            }
        }

        // Note: might want to support other line ending conventions
        if (payload[idx] == '\n') {
            payload[idx] = '\0';
            idx++;
            break;
        }

        idx++;
    }

    // Handle message overflow
    if (idx == max) {
        return -4;
    }

    fh_message_setDataLen(msg, idx);

    return 0;
}

static void
_fh_protocol_destroy_ctx_cli(void **ctx_p)
{
    // NOTE: no dynamic memory within _fh_cli_protocol_ctx
    assert(ctx_p);
    if (*ctx_p) {
        free(*ctx_p);
        *ctx_p = 0;
    }
}

// ############################################################################
// a protocol implementation that converts from ascii message exchange to
// command-line interface with the legacy "cobs encoded frame based" message
// exchange.
//
//
//  * frame format:
//  * <\0><cobs encoded data><\0>
//  *
//  * cobs decoded data:
//  * <busid><ascii msg data><chksum>
//
// ############################################################################
typedef struct {
    uint8_t mt;                 // type of generated ascii msg
    fh_protocol_t *legacy_cobs; // provides legacy framing format
} _fh_legacy_cobs_protocol_ctx;

static int _fh_protocol_encode_legacy_cobs(void *ctx, fh_message_t *msg, fh_stream_t *dst);
static int _fh_protocol_decode_legacy_cobs(void *ctx, fh_message_t *msg, fh_stream_t *src);
static void _fh_protocol_destroy_ctx_legacy_cobs(void **ctx_p);

static fh_protocol_impl LEGACY_COBS_PROTOCOL_IMPL = {.encode = &_fh_protocol_encode_legacy_cobs,
                                                     .decode = &_fh_protocol_decode_legacy_cobs,
                                                     .destroy_ctx = _fh_protocol_destroy_ctx_legacy_cobs};

//  create a new CLI protocol
static fh_protocol_t *
_fh_protocol_new_legacy_cobs(uint8_t mt, size_t max_msg_size)
{
    _fh_legacy_cobs_protocol_ctx *ctx = (_fh_legacy_cobs_protocol_ctx *)calloc(1, sizeof(_fh_legacy_cobs_protocol_ctx));
    assert(ctx);
    ctx->mt = mt;

    ctx->legacy_cobs = fh_frame_protocol_legacy_new(max_msg_size);

    return fh_protocol_new(ctx, LEGACY_COBS_PROTOCOL_IMPL);
}

static int
_fh_protocol_encode_legacy_cobs(void *ctx, fh_message_t *msg, fh_stream_t *dst)
{
    _fh_legacy_cobs_protocol_ctx *self = (_fh_legacy_cobs_protocol_ctx *)ctx;

    uint8_t *data = fh_message_getData(msg);
    uint16_t len = fh_message_dataLen(msg);
    uint8_t mst = fh_message_getSubtype(msg);

    switch (mst) {
    case ASCII_RESP:
        // must be null terminated payload
        assert(data[len - 1] == 0);

        // replace null termination with newline
        if (len > 0 && (data[len - 1] == '\0')) {
            data[len - 1] = '\n';
        }
        break;
    case BINARY_RESP:
        break;
    default:
        return -1;
    }

    return fh_protocol_encode(self->legacy_cobs, msg, dst);
}

static int
_fh_protocol_decode_legacy_cobs(void *ctx, fh_message_t *msg, fh_stream_t *src)
{
    _fh_legacy_cobs_protocol_ctx *self = (_fh_legacy_cobs_protocol_ctx *)ctx;

    // poplate the message with the raw ascii string transported with
    // legacy cobs framing
    fh_protocol_decode(self->legacy_cobs, msg, src);

    // normalize incoming ascii messages by stripping off line endings
    // and adding a null termination.
    //
    // e.g: Incoming ascii message
    //  Original Message:
    //  0000  53 54 4f 50 5f 52 55 4e 0d 0a                    STOP_RUN..
    //  Normalizef Message:
    //  0000  00 01 00 0a 00 00 53 54 4f 50 5f 52 55 4e 00 0d 0a 00 ......STOP_RUN..
    //                                                  ^  x  x  x
    //                                                  |  |  |  |
    //                                              [ added/removed ]

    uint8_t *data = fh_message_getData(msg);
    uint16_t len = fh_message_dataLen(msg);
    int eom = len;

    while (eom >= 1 && (data[eom - 1] == 0x0a || data[eom - 1] == 0x0d || data[eom - 1] == 0x00)) {
        eom--;
    }

    data[eom++] = '\0';

    fh_message_setDataLen(msg, eom);
    fh_message_setType(msg, self->mt);
    fh_message_setSubtype(msg, ASCII_CMD);

    return 0;
}

static void
_fh_protocol_destroy_ctx_legacy_cobs(void **self_p)
{
    assert(self_p);
    if (*self_p) {
        _fh_legacy_cobs_protocol_ctx *self = *self_p;
        fh_protocol_destroy(&(self->legacy_cobs));
        free(self);
        *self_p = NULL;
    }
}
