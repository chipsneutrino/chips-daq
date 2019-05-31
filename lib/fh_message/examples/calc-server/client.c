// ------------------------------------------------------------------
// Implements a client to the example echo server.
// ------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <getopt.h>

// fh_message_lib : messaging library
#include "fh_library.h"

// local
#include "server_api.h"


#define DEFAULT_SERVER_IP "127.0.0.1"

// options
enum protocol
{
    PLAIN,
    COBS_FRAME
};
#define MAX_MESSAGE_SIZE 4096

char *server_ip = DEFAULT_SERVER_IP;       // server ip
uint16_t port = DEFAULT_PORT;              // server port
enum protocol encoding_protocol = PLAIN;   // message encoding scheme
bool trace_protocol = true;                // trace messages on stdout
bool trace_stream = false;                 // trace stream operations on stdout

// forwards
void msg_recv(fh_message_t *msg, void *ctx);


int main(int argc, char *argv[]) {

  int opt;
  while ((opt = getopt(argc, argv, "hp:i:e:t:")) != -1)
      switch (opt) {
      case 'p':
          port = atoi(optarg);
          break;
      case 'i':
          server_ip = optarg;
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
          fprintf(stderr, "Usage: %s [-h] [-p port] [-i ip_addr] [-e 1=plain 2=cobs_frame] [-t 0=none, 1=protocol, "
                          "2=stream, 3=both]\n",
                  argv[0]);
          fprintf(stderr, "    -h  help       Print usage\n");
          fprintf(stderr, "    -p  port       Set server port (DEFAULT=%d)\n", DEFAULT_PORT);
          fprintf(stderr, "    -i  ip_addr    Set server ip address (DEFAULT=%s)\n", DEFAULT_SERVER_IP);
          fprintf(stderr, "    -e  encoding   Set the message encoding (DEFAULT=plain)\n");
          fprintf(stderr, "    -t  trace      Set tracing level (DEFAULT=protocol)\n");
          return 0;
      case '?':
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
          return 1;
      default:
          abort();
      }
  
// connect to server
    fh_connector_t *conn = fh_connector_new_tcp_client(server_ip, port);
    fh_transport_t *transport = fh_transport_new(fh_protocol_new_plain(), fh_connector_connect(conn));
    

    assert(transport);

  // initialze messaging stack
  fh_message_t *msg = fh_message_new();

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

  // ###############################################################
  // Exercise MSG_SERVICE
  // ###############################################################

  // send a GET_STATUS message
  printf("Sending status message...\n");
  fh_message_setType(msg, MSG_SERVICE);
  fh_message_setSubtype(msg, MS_STATUS);
  fh_transport_send(transport, msg);
  fh_transport_receive(transport, msg);
  uint8_t status = *fh_message_getData(msg);
  printf("OK: Got status message:[%d]\n\n", status);

 // send a MS_ECHO message
  printf("Sending echo message...\n");
  fh_message_setType(msg, MSG_SERVICE);
  fh_message_setSubtype(msg, MS_ECHO);
  fh_message_setData(msg, (uint8_t*)"Hello World!", 12);
  fh_transport_send(transport, msg);
  fh_transport_receive(transport, msg);
  char tmp[fh_message_dataLen(msg) + 1];
  memcpy(tmp, fh_message_getData(msg),fh_message_dataLen(msg) );
  printf("OK: Got echo message:[%s]\n\n", tmp);

  // send a MS_ECHO message, more compact initialization
  printf("Sending echo message...\n");
  fh_message_init_full(msg, MSG_SERVICE, MS_ECHO, (uint8_t*)"Lorem ipsum", 11);
  fh_transport_send(transport, msg);
  fh_transport_receive(transport, msg);
  char tmp2[fh_message_dataLen(msg) + 1];
  memcpy(tmp2, fh_message_getData(msg),fh_message_dataLen(msg) );
  printf("OK: Got echo message:[%s]\n\n", tmp2);

  // ###############################################################
  // Exercise CALC_SERVICE
  // ###############################################################

 // send a CALC_ADD message
  {
  printf("Sending ADD message...\n");
  uint8_t nums[12];
  encode_int32(nums, 111, 0);
  encode_int32(nums, 222, 4);
  encode_int32(nums, 333, 8);
  fh_message_init_full(msg, CALC_SERVICE, CS_ADD, nums, 12);
  fh_transport_send(transport, msg);
  fh_transport_receive(transport, msg);
  printf("OK: %d + %d + %d = %d\n\n", 111, 222, 333, extract_int32(msg, 0));
  }

  // send a CALC_SUBTRACT message
  {
  printf("Sending SUBTRACT message...\n");
  uint8_t nums[8];
  encode_int32(nums, 1000, 0);
  encode_int32(nums, 343, 4);
  fh_message_init_full(msg, CALC_SERVICE, CS_SUBTRACT, nums, 8);
  fh_transport_send(transport, msg);
  fh_transport_receive(transport, msg);
  printf("OK: %d - %d = %d\n\n", 1000, 343, extract_int32(msg, 0));
  }

  // send a CALC_MULTIPLY message
  {
  printf("Sending MULTIPLY message...\n");
  uint8_t nums[12];
  encode_int32(nums, 33, 0);
  encode_int32(nums, 56, 4);
  encode_int32(nums, 88, 8);
  fh_message_init_full(msg, CALC_SERVICE, CS_MULTIPLY, nums, 12);
  fh_transport_send(transport, msg);
  fh_transport_receive(transport, msg);
  printf("OK: %d + %d + %d = %d\n\n", 33, 56, 88, extract_int32(msg, 0));
  }

  // send a CALC_DIVIDE message
  {
  printf("Sending DIVIDE message...\n");
  uint8_t nums[8];
  encode_int32(nums, 400, 0);
  encode_int32(nums, 4, 4);
  fh_message_init_full(msg, CALC_SERVICE, CS_DIVIDE, nums, 8);
  fh_transport_send(transport, msg);
  fh_transport_receive(transport, msg);
  printf("OK: %d / %d = %d\n\n", 400, 4, extract_int32(msg, 0));
  }


  printf("waiting to disconnect...\n\n");
  sleep(5);


  // send a MS_CLOSE message
  printf("Client disconnecting...\n");
  fh_message_setType(msg, MSG_SERVICE);
  fh_message_setSubtype(msg, MS_CLOSE);
  fh_message_setData(msg, NULL, 0);
  fh_transport_send(transport, msg);
  fh_transport_receive(transport, msg);
  printf("OK: Client disconnect.\n\n");
  


  printf("Client exiting...\n");
  fh_transport_destroy(&transport);
  fh_message_destroy(&msg);

  return 0;
}






