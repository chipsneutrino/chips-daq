/**
 * BBBTest - A simple "beaglebone" test server to see if the
 * fh_library is working
 * 
 * NOTE: THIS IS A TEST IMPLEMENTATION!!!
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "fh_library.h"
#include "DAQ_bbb_api.h"

bool init();
void destroy();
int handle_command(uint32_t count);
fh_service_t * ms_new();

// ###########################################################################
// members
// ###########################################################################
bool connected;                // client connection status
int client_fd;                 // client connection file descriptor
fh_transport_t *transport;     // encapsulates message exchange
fh_dispatch_t *dispatcher;     // encapsulates message dispatching
fh_service_t *msg_srvc;        // pre-allocated message instance
fh_message_t *msgin;           // bare-bones service to manage client connection

// options
uint16_t port = DEFAULT_PORT;  // server port
bool trace_protocol = true;    // trace messages on stdout
bool trace_stream = false;     // trace stream operations on stdout

int main(int argc, char *argv[]) {

    printf("starting server on port %d...\n", port);
    if (!init()) {
        printf("Initialization Error, Server exiting.\n");
    }

    // loop on client messages
    bool fault = false;
    uint32_t count = 0;
    while(connected && !fault){
        int status = handle_command(count++);
        fault = (status != 0);
    }

    // unrequested exit
    if (fault) {
        printf("control channel fault, Server exiting.\n");
    }

    // client-requested exit
    if (!connected) {
        printf("client closed connection, Server exiting.\n");
    }

    destroy();

    return 1;
}

// Initialise members
bool init() {
    // open socket/wait for client connection
    fh_connector_t *conn = fh_connector_new_tcp_server(port);
    transport = fh_transport_new(fh_frame_protocol_new(MAX_MESSAGE_SIZE, FP_VERSION_2), fh_connector_connect(conn));

    if (!transport) {
        return false;
    }
    else {
        connected = true;
    }

    // Initialse messaging stack
    dispatcher = fh_dispatch_new();
    msgin = fh_message_new();

    // trace message xchange on stdout ?
    if (trace_protocol) {
        fh_transport_enable_protocol_trace(transport, stdout);
    }
    if (trace_stream) {
        fh_transport_enable_stream_trace(transport, stdout);
    }

    // initialize message service register with dispatcher
    msg_srvc = ms_new();
    fh_dispatch_register_service(dispatcher, msg_srvc);

    return true;
}

// Destructor
void destroy() {
    fh_transport_destroy(&transport);
    fh_dispatch_destroy(&dispatcher);
    fh_message_destroy(&msgin);
    fh_service_destroy(&msg_srvc);
}

// Message Handler
int handle_command(uint32_t count) {
    int status = fh_transport_receive(transport, msgin);
    printf(">>Processing Command [%d]\n", count++);
    if (status != 0) {
        printf("FAULT: error receiving message [status:%d]\n", status);
        return -1;
    }
    status = fh_dispatch_handle(dispatcher, msgin, transport);
    if (status != 0) {
        printf("FAULT: error handling message [status:%d]\n", status);
        return -1;
    }
    printf("<<\n\n");

    return 0;
}

// ##################################################################
// MESSAGE_SERVER functions
// ##################################################################
// The msg server implements the following commands:
//
// [subcode] <function desc>
// payload in:
// payload out:
//
//
//
// [1] <status>
// in: n/a
// out: uint8 : 1=ready, 2=fault
//
// [2] <echo>
// in: uint8[]  : data to echo
// out: uint8[] : copy of in
//
// [3] <close>
// in: n/a
// out: uint_8[] : "GOODBYE"
//
//
// ------------------------------------------------------------------

//forwards
int ms_status(void *ctx, fh_message_t *msgin, fh_transport_t *transport);
int ms_echo(void *ctx, fh_message_t *msgin, fh_transport_t *transport);
int ms_close(void *ctx, fh_message_t *msgin, fh_transport_t *transport);

fh_service_t * ms_new() {
    fh_service_t *srvc = fh_service_new(MSG_SERVICE, NULL);       // new service handling typecode MSG_SERVICE
    fh_service_register_function(srvc, MS_STATUS, &ms_status);    // bind "status" function to subcode MS_STATUS
    fh_service_register_function(srvc, MS_ECHO, &ms_echo);        // bind "echo" functionto subcode MS_ECHO
    fh_service_register_function(srvc, MS_CLOSE, &ms_close);      // bind "close" functionto subcode MS_CLOSE
    return srvc;
}

int ms_status(void *ctx, fh_message_t *msgin, fh_transport_t *transport) {
    uint8_t out[] = {1}; // OK
    fh_message_setData(msgin, out, 1);
    return fh_transport_send(transport, msgin);
}

int ms_echo(void *ctx, fh_message_t *msgin, fh_transport_t *transport) {
    int size = fh_message_dataLen(msgin);
    uint8_t *original = fh_message_getData(msgin);
    uint8_t copy[size];
    memcpy(copy, original, size);
    fh_message_setData(msgin, copy, size);
    return fh_transport_send(transport, msgin);
}

int ms_close(void *ctx, fh_message_t *msgin, fh_transport_t *transport) {
    fh_message_setData(msgin, (uint8_t *)"GOODBYE", 7);
    int status = fh_transport_send(transport, msgin);

    if (close(client_fd) != 0) {
        printf("Error closing client connection\n");
    }
    client_fd = 0;
    connected = false;
    return status;
}
