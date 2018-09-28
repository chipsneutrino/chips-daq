/**
 * udaq_peer.h
 *
 * Provides a local interface to a remote udaq.
 */

#include "standard_inc.h"
#include <pthread.h>

#include "udaq_comms_api.h"
#include "udaq_peer.h"

// instance data
struct _udaq_peer_t {
    uint8_t channel_id;
    peer_state state;
    fh_connector_t *connector; // used to establish connection
    fh_transport_t *transport; // client connection to udaq msg interface
    fh_message_t *msg;         // message structure used for comms
    bool trace_protocol;       // debugging mode for msg layer
    FILE *trace_out;           // debugging destination for msg layer
};

udaq_peer_t *
udaq_peer_new(uint8_t channel_id, fh_connector_t *connector)
{
    udaq_peer_t *self = (udaq_peer_t *)calloc(1, sizeof(udaq_peer_t));
    assert(self);

    self->channel_id = channel_id;
    self->state = DISCONNECTED;
    self->connector = connector;
    self->transport = NULL;
    self->msg = fh_message_new();

    self->trace_protocol = false;
    self->trace_out = NULL;
    ;

    return self;
}

void
udaq_peer_destroy(udaq_peer_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        udaq_peer_t *self = *self_p;

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
udaq_peer_trace_protocol(udaq_peer_t *self, FILE *out)
{
    self->trace_protocol = true;
    self->trace_out = out;
    if (self->transport) {
        fh_transport_enable_protocol_trace(self->transport, self->trace_out);
    }
}

peer_state
udaq_peer_get_state(udaq_peer_t *self)
{
    return self->state;
}

bool
udaq_peer_connect(udaq_peer_t *self)
{
   // printf("udaq connect...\n");
    fh_stream_t *stream = fh_connector_connect(self->connector);

    if (stream) {
        self->transport = fh_transport_new(fh_protocol_new_plain(), stream);
        self->state = CONNECTED;

        if (self->trace_protocol) {
            fh_transport_enable_protocol_trace(self->transport, self->trace_out);
        }

       // printf("udaq connect OK.\n");
    }
    else {
        printf("udaq connect ERROR.\n");
    }
    return self->transport != NULL;
}

bool
udaq_peer_start_run(udaq_peer_t *self)
{
    printf("udaq start run...\n");

    // send an ECS_START_RUN message
    fh_message_setType(self->msg, EXP_CTRL_SERVICE);
    fh_message_setSubtype(self->msg, ECS_START_RUN);
    fh_transport_send(self->transport, self->msg);
    fh_transport_receive(self->transport, self->msg);
    uint8_t status = *fh_message_getData(self->msg);
    //printf("OK: start run:[%d]\n\n", status);
    //printf("udaq start run OK.\n");

    return status > 0;
}

bool
udaq_peer_stop_run(udaq_peer_t *self)
{
    //printf("udaq stop run...\n");

    // send an ECS_STOP_RUN message
    fh_message_setType(self->msg, EXP_CTRL_SERVICE);
    fh_message_setSubtype(self->msg, ECS_STOP_RUN);
    fh_transport_send(self->transport, self->msg);
    fh_transport_receive(self->transport, self->msg);
    uint8_t status = *fh_message_getData(self->msg);
    //printf("OK: stop run:[%d]\n\n", status);
    //printf("udaq stop run OK.\n");

    return status > 0;
}

bool
udaq_peer_poll_page(udaq_peer_t *self, page_handler *handler, size_t *length)
{
    // printf("udaq poll page...\n");

    fh_message_setType(self->msg, DATA_ACCESS_SERVICE);
    fh_message_setSubtype(self->msg, DS_POLL_PAGE);
    fh_transport_send(self->transport, self->msg);
    fh_transport_receive(self->transport, self->msg);
    // printf("OK: Got Page:[%d]\n\n", fh_message_dataLen(self->msg));
    // printf("udaq poll page OK.\n");

    size_t len = fh_message_dataLen(self->msg);
    if (len > 0) {
        (*(handler->handle_page))(handler->ctx, self->channel_id, fh_message_getData(self->msg), len);
    }
    *length = len;

    return true;
}
