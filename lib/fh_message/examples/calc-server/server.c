
// ------------------------------------------------------------------
// server.c
// Implements an example server using the fh_message library.
//
// This server provides two services:
//
// MESSAGE_SERVICE: Provides basic functions related to the client
//                  connection.
//
// CALC_SERVICE: Provides add, subtract, multiply, divide functions.
//
// ------------------------------------------------------------------


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

// fh_message_lib : messaging library
#include "fh_library.h"

// local
#include "server_api.h"
#include "calc_service.h"


// ###########################################################################
// forwards
// ###########################################################################
bool init();
void destroy();
int handle_command(uint32_t count);
fh_service_t * ms_new();


// ###########################################################################
// message encoding options
// ###########################################################################
enum protocol
{
    PLAIN,
    COBS_FRAME
};
#define MAX_MESSAGE_SIZE 4096


// ###########################################################################
// members
// ###########################################################################
bool connected;                // client connectio status
int client_fd;                 // client connection file descriptor
fh_transport_t *transport;     // encapsulates messege exchange
fh_dispatch_t *dispatcher;     // encasulates message dispatching
fh_service_t *msg_srvc;        // pre-allocated message instance
fh_message_t *msgin;           // bare-bones service to manage client connection
calc_service_t *calc_srvc;     // a calculator service

// options
uint16_t port = DEFAULT_PORT;       // server port
enum protocol encoding_protocol = PLAIN; // message encoding scheme
bool trace_protocol = true;        // trace messages on stdout
bool trace_stream = false;          // trace stream operations on stdout


// main
int
main(int argc, char *argv[])
{

    int opt;
    while ((opt = getopt(argc, argv, "hp:e:t:")) != -1)
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 't': {
          trace_protocol = false;
          trace_stream = false;
          int val = atoi(optarg);
          switch (val) {
          case 0:
              break;
          case 1:
              trace_protocol = true;
              break;
          case 2:
              trace_stream = true;
              break;
          case 3:
              trace_protocol = true;
              trace_stream = true;
              break;
          default:
              fprintf(stderr, "Illegal option -t: %d\n", val);
              return 1;
          }
          break;
      }
        case 'e': {
            int val = atoi(optarg);
            switch (val) {
            case 1:
                encoding_protocol = PLAIN;
                break;
            case 2:
                encoding_protocol = COBS_FRAME;
                break;
            default:
                fprintf(stderr, "Illegal option -e: %d\n", val);
                return 1;
            }
            break;
        }
        case 'h':
            fprintf(stderr, "Usage: %s [-h] [-p port] [-e 1=plain 2=cobs_frame] [-t 0=none, 1=protocol, 2=stream, 3=both]\n", argv[0]);
            fprintf(stderr, "    -h  help       Print usage\n");
            fprintf(stderr, "    -p  port       Set server port (DEFAULT=%d)\n", DEFAULT_PORT);
            fprintf(stderr, "    -e  encoding   Set the message encoding (DEFAULT=plain)\n");
            fprintf(stderr, "    -t  trace      Set tracing level (DEFAULT=protocol)\n");
            return 0;
        case '?':
            fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            return 1;
        default:
            abort();
        }

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
}

// init members
bool
init()
{

    // open socket/wait for client connection
    fh_connector_t *conn = fh_connector_new_tcp_server(port);
    transport = fh_transport_new(fh_protocol_new_plain(), fh_connector_connect(conn));

    if (!transport) {
        return false;
    }
    else {
        connected = true;
    }

    // initialze messaging stack
    dispatcher = fh_dispatch_new();
    msgin = fh_message_new();

    if (encoding_protocol == COBS_FRAME) {
        fh_transport_set_protocol(transport, fh_frame_protocol_new(MAX_MESSAGE_SIZE, FP_VERSION_1));
    }

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

    // initialize cal service and register with dispatcher
    calc_srvc = calc_new(CALC_SERVICE);
    calc_register_service(calc_srvc, dispatcher);

    return true;
}

// destructor
void
destroy()
{
    fh_transport_destroy(&transport);
    fh_dispatch_destroy(&dispatcher);
    fh_message_destroy(&msgin);
    fh_service_destroy(&msg_srvc);
    calc_destroy(&calc_srvc);
}




int
handle_command(uint32_t count)
{
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

fh_service_t *
ms_new()
{
    fh_service_t *srvc = fh_service_new(MSG_SERVICE, NULL);       // new service handling typecode MSG_SERVICE
    fh_service_register_function(srvc, MS_STATUS, &ms_status);    // bind "status" function to subcode MS_STATUS
    fh_service_register_function(srvc, MS_ECHO, &ms_echo);        // bind "echo" functionto subcode MS_ECHO
    fh_service_register_function(srvc, MS_CLOSE, &ms_close);      // bind "close" functionto subcode MS_CLOSE
    return srvc;
}

int
ms_status(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    uint8_t out[] = {1}; // OK
    fh_message_setData(msgin, out, 1);
    return fh_transport_send(transport, msgin);
}

int
ms_echo(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    int size = fh_message_dataLen(msgin);
    uint8_t *original = fh_message_getData(msgin);
    uint8_t copy[size];
    memcpy(copy, original, size);
    fh_message_setData(msgin, copy, size);
    return fh_transport_send(transport, msgin);
}

int
ms_close(void *ctx, fh_message_t *msgin, fh_transport_t *transport)
{
    fh_message_setData(msgin, (uint8_t *)"GOODBYE", 7);
    int status = fh_transport_send(transport, msgin);

    if (close(client_fd) != 0) {
        printf("Error closing client connection\n");
    }
    client_fd = 0;
    connected = false;
    return status;
}
