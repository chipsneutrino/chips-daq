/**
 * fh_peer.c
 *
 * provide a local interface to a remote field hub.
 */
#include "standard_inc.h"
#include <pthread.h>

#include "bbb_comms_api.h"
#include "fh_peer.h"

// instance data
struct _fh_peer_t {
    uint32_t fh_id;
    peer_state state;
    fh_connector_t *connector; // used to establish connection
    fh_transport_t *transport; // client connection to field hub msg interface
    fh_message_t *msg;         // message structure used for comms
    bool trace_protocol;       // debugging mode for msg layer
    bool trace_stream;         // debugging mode for stream layer
    FILE *trace_out;           // debugging destination for msg trace
};

fh_peer_t *
fh_peer_new(uint32_t fh_id, fh_connector_t *connector)
{
    fh_peer_t *self = (fh_peer_t *)calloc(1, sizeof(fh_peer_t));
    assert(self);

    self->fh_id = fh_id;
    self->state = DISCONNECTED;
    self->connector = connector;
    self->transport = NULL;
    self->msg = fh_message_new();
    self->trace_protocol = false;
    self->trace_stream = false;
    self->trace_out = NULL;

    return self;
}

void
fh_peer_destroy(fh_peer_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_peer_t *self = *self_p;

        fh_connector_destroy(&(self->connector)); // destroy connector
        if (self->transport) {
            fh_transport_destroy(&(self->transport)); // destroy transport
        }
        fh_message_destroy(&(self->msg)); // destroy message

        free(self);
        *self_p = NULL;
    }
}

void
fh_peer_trace_protocol(fh_peer_t *self, FILE *out)
{
    self->trace_protocol = true;
    self->trace_out = out;
    if (self->transport) {
        fh_transport_enable_protocol_trace(self->transport, self->trace_out);
    }
}

void
fh_peer_trace_stream(fh_peer_t *self, FILE *out)
{
    self->trace_stream = true;
    self->trace_out = out;
    if (self->transport) {
        fh_transport_enable_stream_trace(self->transport, self->trace_out);
    }
}

uint32_t
fh_peer_get_id(fh_peer_t *self)
{
    return self->fh_id;
}

peer_state
fh_peer_get_state(fh_peer_t *self)
{
    return self->state;
}

bool
fh_peer_connect(fh_peer_t *self)
{
    printf("bbb connect...\n");
    fh_stream_t *stream = fh_connector_connect(self->connector);

    if (stream) {
        self->transport = fh_transport_new(fh_protocol_new_plain(), stream);

        self->state = CONNECTED;
        if (self->trace_protocol) {
            fh_transport_enable_protocol_trace(self->transport, self->trace_out);
        }
        if (self->trace_stream) {
            fh_transport_enable_stream_trace(self->transport, self->trace_out);
        }
        printf("bbb connect OK.\n");
    }
    else {
        printf("bbb connect ERROR.\n");
    }
    return self->transport != NULL;
}

bool
fh_peer_configure(fh_peer_t *self)
{
    printf("bbb configure..\n");

    // send an ECS_START_RUN message
    fh_message_setType(self->msg, FH_CTRL_SERVICE);
    fh_message_setSubtype(self->msg, FH_CONFIGURE);
    fh_transport_send(self->transport, self->msg);
    fh_transport_receive(self->transport, self->msg);
    uint8_t status = *fh_message_getData(self->msg);
    printf("OK: configure:[%d]\n\n", status);
    printf("bbb configure OK.\n");

    return status > 0;
}

bool
fh_peer_start_run(fh_peer_t *self)
{
    printf("bbb start run...\n");

    // send an ECS_START_RUN message
    fh_message_setType(self->msg, FH_CTRL_SERVICE);
    fh_message_setSubtype(self->msg, FH_START_RUN);
    fh_transport_send(self->transport, self->msg);
    fh_transport_receive(self->transport, self->msg);
    uint8_t status = *fh_message_getData(self->msg);
    printf("OK: start run:[%d]\n\n", status);
    printf("bbb start run OK.\n");

    return status > 0;
}

bool
fh_peer_stop_run(fh_peer_t *self)
{
    printf("bbb stop run...\n");

    // send an ECS_STOP_RUN message
    fh_message_setType(self->msg, FH_CTRL_SERVICE);
    fh_message_setSubtype(self->msg, FH_STOP_RUN);
    fh_transport_send(self->transport, self->msg);
    fh_transport_receive(self->transport, self->msg);
    uint8_t status = *fh_message_getData(self->msg);
    printf("OK: stop run:[%d]\n\n", status);
    printf("bbb stop run OK.\n");

    return status > 0;
}

bool
fh_peer_poll_page(fh_peer_t *self, page_handler *handler, size_t *length)
{
    // printf("field hub poll page...\n");
    fh_message_setType(self->msg, DATA_ACCESS_SERVICE);
    fh_message_setSubtype(self->msg, DS_POLL_PAGE);
    fh_transport_send(self->transport, self->msg);
    fh_transport_receive(self->transport, self->msg);
    // printf("OK: Got Page:[%d]\n\n", fh_message_dataLen(self->msg));
    // printf("bbb poll page OK.\n");

    size_t len = fh_message_dataLen(self->msg);
    if (len > 0) {
        (*(handler->handle_page))(handler->ctx, self->fh_id, fh_message_getData(self->msg), len);
    }

    *length = len;

    return true;
}
