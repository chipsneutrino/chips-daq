/**
 * fh_transport_test_main.c
 *
 * Application for testing message transport implementations.
 */
#include "../../fh_classes.h"

#include <getopt.h>
#include <termios.h>
#include <errno.h>


#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 6000
#define DEFAULT_SERIAL_DEV "/dev/cu.usbserial-AI05605R"

 #define streq(s1,s2)    (!strcmp ((s1), (s2)))

 // forwards
void print_config();

fh_transport_t * create_transport();
fh_protocol_t * create_protocol();
fh_connector_t * create_connector();
int set_interface_attribs(int fd, int speed);
int open_serial_dev(char *dev);


enum role_option { CLIENT, SERVER };
enum protocol_option { PLAIN, COBS_FRAME_V1, COBS_FRAME_V2 };
enum stream_option { SERIAL, TCPIP };

typedef struct {
    enum role_option role;         // client or server
    enum stream_option conn;       // connection type
    char *ip;                      // ip address of server (TCPIP, CLIENT)
    int port;                      // port of server (TCPIP)
    char *serial_dev;              // serial device (SERIAL)
    enum protocol_option protocol; // protocol
    bool trace_protocol;           // trace messages?
    bool trace_stream;             // trace messages I/O?
    float burst_err_rate;          // rate of inserted bit-error events
} config_t;

config_t config;

int main(int argc, char *argv[]) {

    // defaults
    config.role = SERVER;
    config.conn = TCPIP;
    config.ip = DEFAULT_SERVER_IP;
    config.port = DEFAULT_PORT;
    config.serial_dev = DEFAULT_SERIAL_DEV;
    config.protocol = PLAIN;

  int opt;
  while ((opt = getopt(argc, argv, "hr:c:e:p:i:d:t:b:")) != -1)
    switch (opt) {
         case 'r':
      if (streq(optarg, "client")) {
        config.role = CLIENT;
      } else if (streq(optarg, "server")) {
        config.role = SERVER;
      } else {
        fprintf(stderr, "Illegal option -c: %s\n", optarg);
        return 1;
      }
      break;
    case 'c':
      if (streq(optarg, "tcpip")) {
        config.conn = TCPIP;
      } else if (streq(optarg, "serial")) {
        config.conn = SERIAL;
      } else {
        fprintf(stderr, "Illegal option -c: %s\n", optarg);
        return 1;
      }
      break;
    case 'e':
      if (streq(optarg, "plain")) {
        config.protocol = PLAIN;
      } else if (streq(optarg, "cobs_frame_v1")) {
        config.protocol = COBS_FRAME_V1;
      } else if (streq(optarg, "cobs_frame_v2")) {
        config.protocol = COBS_FRAME_V2;
      } else {
        fprintf(stderr, "Illegal option -e: %s\n", optarg);
        return 1;
      }
      break;
    case 'p':
      config.port = atoi(optarg);
      break;
    case 'i':
      config.ip = optarg;
      break;
    case 'd':
      config.serial_dev = optarg;
      break;
    case 't': {
      config.trace_protocol = false;
      config.trace_stream = false;
      int val = atoi(optarg);
      switch (val) {
      case 0:
        break;
      case 1:
        config.trace_protocol = true;
        break;
      case 2:
        config.trace_stream = true;
        break;
      case 3:
        config.trace_protocol = true;
        config.trace_stream = true;
        break;
      default:
        fprintf(stderr, "Illegal option -t: %d\n", val);
        return 1;
      }
      break;
      }
      case 'b':
        config.burst_err_rate = atof(optarg);
        break;
      case 'h':
          fprintf(stderr, "Usage: %s -c tcpip [-r role] [-p port] [-i ip_addr] [-e protocol] [-t trace]\n", argv[0]);
          fprintf(stderr, "Usage: %s -c serial [-r role] [-d device][-e protocol] [-t trace]\n", argv[0]);
          fprintf(stderr, "Usage: %s [-h]\n", argv[0]);
          fprintf(stderr, "    -h  help         Print usage\n");
          fprintf(stderr, "    -c  connection   Set connection type (tcpip, serial, DEFAULT=tcpip)\n");
          fprintf(stderr, "    -r  role         Set role (client, server, DEFAULT=server)\n");
          fprintf(stderr, "    -p  port         Set server port (DEFAULT=%d)\n", DEFAULT_PORT);
          fprintf(stderr, "    -i  ip_addr      Set server ip address (DEFAULT=%s)\n", DEFAULT_SERVER_IP);
          fprintf(stderr, "    -d  device       Set the serial device (DEFAULT=%s\n", DEFAULT_SERIAL_DEV);
          fprintf(stderr, "    -e  encoding     Set the message encoding (plain, cobs_frame_v1, cobs_frame_v2, DEFAULT=plain)\n");
          fprintf(stderr, "    -t  trace        Sets the tracing level (trace_option 0=none 1=protocol 2=stream 3=both DEFAULT=none)\n");
          fprintf(stderr, "    -b  trace        Sets the burst error rate (DEFAULT=0)\n");
          return 0;
      case '?':
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
          return 1;
      default:
          abort();
      }

      print_config();

     

      if(config.role == SERVER)
      {
         fh_transport_test_server_th(create_transport(), stdout);
      }
      else if (config.role == CLIENT)
      {
         fh_transport_test_client_th(create_transport(), stdout);
      }
      else
      {
        fprintf(stderr, "Bad role: %d\n", config.role);
      }

      return 1;

}

void print_config() {
    printf("Starting transport test:\n");
 
  switch (config.role) {
  case CLIENT:
    printf("Role: client\n");
    break;
  case SERVER:
    printf("Role: server\n");
    break;
  }

  switch (config.protocol) {
  case PLAIN:
    printf("Protocol: PLAIN\n");
    break;
  case COBS_FRAME_V1:
    printf("Protocol: COBS_FRAME_V1\n");
    break;
  case COBS_FRAME_V2:
    printf("Protocol: COBS_FRAME_V2\n");
    break;
  }

  switch (config.conn) {
  case TCPIP:
    printf("Connection: TCPIP [%s:%d]\n", config.ip, config.port);
    break;
  case SERIAL:
    printf("Connection: SERIAL [%s]\n", config.serial_dev);
    break;
  }

  printf("Trace Protocol: [%s]\n", config.trace_protocol ? "true" : "false");
  printf("Trace Stream: [%s]\n", config.trace_stream ? "true" : "false");
  printf("Burst Error Raye: [%f]\n", config.burst_err_rate);

  printf("\n");
}

fh_transport_t *
create_transport() {
  fh_connector_t *connector = create_connector();
  fh_protocol_t *protocol = create_protocol();

  fh_stream_t *stream = fh_connector_connect(connector);
  fh_connector_destroy(&connector);

  fh_transport_t *transport =
      fh_transport_new(protocol, fh_error_stream_new(stream, config.burst_err_rate, stdout));


      if(config.trace_stream)
      {
        fh_transport_enable_stream_trace(transport, stdout);
      }

      if(config.trace_protocol)
      {
        fh_transport_enable_protocol_trace(transport, stdout);
      }

      return transport;
}

fh_connector_t *
create_connector() {
  switch (config.conn) {

  case TCPIP: {
    switch (config.role) {
    case SERVER: {
      return fh_connector_new_tcp_server(config.port);
      break;
    }
    case CLIENT: {
      return fh_connector_new_tcp_client(config.ip, config.port);
      break;
    }
    default:
      fprintf(stderr, "Bad role: %d\n", config.role);
      return NULL;
    }
  }

  case SERIAL: {
    int fd = open_serial_dev(config.serial_dev);
    return fh_connector_new_file(fd, fd);
    break;
  }
  default:
    fprintf(stderr, "Bad connection type: %d\n", config.conn);
    return NULL;
  }
}

fh_protocol_t *
create_protocol()
{
    switch(config.protocol)
    {
        case PLAIN:
        return fh_protocol_new_plain();
        break;
        case COBS_FRAME_V1:
        return fh_frame_protocol_new(4096, FP_VERSION_1);
        case COBS_FRAME_V2:
        return fh_frame_protocol_new(4096, FP_VERSION_2);
        break;

        default:
         fprintf(stderr, "Bad protocol type: %d\n", config.protocol);
    return NULL;
    }
}

int
set_interface_attribs(int fd, int speed) {

  printf("set_interface_attribs\n");
  fflush(stdout);

    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
        printf ("error %d from tcgetattr\n", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; //8-bit chars
    // disable IGNBRK for mismatched speed tests
    // otherwise receive break as \000 chars
    //tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0; //no signaling chars, no echo
    // no canonical processing

    tty.c_oflag = 0; //no remapping, no delays, raw output
    //tty.c_cc[VMIN] = 1; //read doesn't block
    //tty.c_cc[VTIME] = 5; //0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); //shut off xon/xoff ctrl
    tty.c_iflag &= ~(IGNCR | INLCR | ICRNL);//don't translate carriage returns to newlines or vice versa
    //definitely don't ignore carriage returns

    tty.c_cflag |= (CLOCAL | CREAD);//ignore modem ctrls
    //enable reading

    tty.c_cflag &= ~(PARENB | PARODD); //shut off parity
    //  tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB; // single stop bit
    //  tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        printf ("error %d from tcsetattr\n", errno);
        return -1;
    }
    return 0;
}

int open_serial_dev(char *dev) {
  // char *dev = "/dev/tty.usbserial-AI05605R";
  printf("opening %s\n", dev);

  // int fd = open(dev, O_RDWR | O_NOCTTY);// | O_NDELAY);
  // int fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
  int fd = open(dev, O_RDWR);

  if (fd < 0) {
    printf("error %d opening %s: %s\n", errno, dev, strerror(errno));
    return -1;
  }

  if (set_interface_attribs(fd, B9600) < 0)
    return -1;

  return fd;
}



/*
#ifdef SERVER
int
main(int argc, char *argv[])
{
#ifdef TCPIP
    fh_connector_t *connector = fh_connector_new_tcp_server(15555);
    fh_protocol_t *protocol = fh_frame_protocol_new(4096, FP_VERSION_2);
#endif

    _server_th(connector, protocol, NULL);

    return 1;
}
#endif

#ifdef CLIENT
int
main(int argc, char *argv[])
{
#ifdef TCPIP
    fh_connector_t *connector = fh_connector_new_tcp_client("127.0.0.1", 15555);
    fh_protocol_t *protocol = fh_frame_protocol_new(4096, FP_VERSION_2);
#endif
#ifdef SERIAL
    fh_connector_t *connector = fh_connector_new_tcp_client("127.0.0.1", 15555);
    fh_protocol_t *protocol = fh_frame_protocol_new(4096, FP_VERSION_2);
#endif

    _client_th(connector, protocol, NULL);

    return 1;
}
#endif
*/


