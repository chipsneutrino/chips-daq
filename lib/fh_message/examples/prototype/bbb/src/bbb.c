/**
 * bbb.c
 *
 * Prototype of the field hub application.
 *
 **/
#include <assert.h>
#include <getopt.h>
#include <string.h>

#include "fh_library.h"

#include "cobs_frame_protocol.h"
#include "fh_connector.h"

#include "bbb_comms_api.h"
#include "sim_bbb.h"

// forwards
bool add_udaq_config(char *config_str);
void print_config();

// options
typedef enum { PLAIN, COBS_FRAME } protocol;

// defaults
#define DEFAULT_UDAQ_IP "127.0.0.1"
#define DEFAULT_PROTOCOL_ENCODING PLAIN
#define DEFAULT_BUFFER_SIZE 16
#define DEFAULT_PAGE_SIZE 10

// holds config data for an attached udaq
typedef struct {
    char *ip;
    uint16_t port;
    uint8_t thread_group;       // binds udaq service to a particular thread
    protocol encoding_protocol; // message encoding scheme
    bool trace_protocol;        // trace messages on stdout
    bool trace_stream;          // trace stream operations on stdout

} udag_config_t;

udag_config_t *
make_config(char *ip, uint16_t port, uint8_t thread_group, protocol encoding_protocol, bool trace_protocol,
            bool trace_stream)
{
    udag_config_t *self = (udag_config_t *)calloc(1, sizeof(udag_config_t));
    assert(self);

    size_t ip_len = strlen(ip);
    self->ip = calloc(1, ip_len);
    strncpy(self->ip, ip, ip_len);

    self->port = port;

    self->thread_group = thread_group;

    self->encoding_protocol = encoding_protocol;
    self->trace_protocol = trace_protocol;
    self->trace_stream = trace_stream;

    return self;
}

void
destroy_config(udag_config_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        udag_config_t *self = *self_p;
        free(self->ip);
        free(self);
        *self_p = NULL;
    }
}

// field hub config
#define MAX_ATTACHED_UDAQS 16
typedef struct {
    udag_config_t *udaq_conf[MAX_ATTACHED_UDAQS]; // pointer to config structures
    size_t num_config_udaqs;                      // number of configured udaqs
    uint16_t port;                                // port to serve sdaq interface on
    protocol encoding_protocol;                   // protocol for sdaq service
    uint8_t buffer_sz;                            // buffer size (bits)
    uint8_t page_sz;                              // buffer page size (bits)
    bool trace_protocol;                          // trace sdaq service messages on stdout
    bool trace_stream;                            // trace sdaq stream operations on stdout
} config;

config bbb_config;

int
main(int argc, char *argv[])
{

    bbb_config.buffer_sz = DEFAULT_BUFFER_SIZE;
    bbb_config.page_sz = DEFAULT_PAGE_SIZE;
    bbb_config.port = DEFAULT_BBB_COMMS_PORT;
    bbb_config.encoding_protocol = DEFAULT_PROTOCOL_ENCODING;
    bbb_config.trace_protocol = false;
    bbb_config.trace_stream = false;

    for (int i = 0; i < MAX_ATTACHED_UDAQS; i++) {
        bbb_config.udaq_conf[i] = NULL;
    }

    // populate global bbb_config struct from command line args
    int opt;
    char *udaq_options[MAX_ATTACHED_UDAQS];
    int udaq_opt_count = 0;
    while ((opt = getopt(argc, argv, "hb:z:p:e:t:u:")) != -1)
        switch (opt) {
        case 'b':
            bbb_config.buffer_sz = atoi(optarg);
            break;
        case 'z':
            bbb_config.page_sz = atoi(optarg);
            break;
        case 'p':
            bbb_config.port = atoi(optarg);
            break;
        case 't': {
            bbb_config.trace_protocol = false;
            bbb_config.trace_stream = false;
            int val = atoi(optarg);
            switch (val) {
            case 0:
                break;
            case 1:
                bbb_config.trace_protocol = true;
                break;
            case 2:
                bbb_config.trace_stream = true;
                break;
            case 3:
                bbb_config.trace_protocol = true;
                bbb_config.trace_stream = true;
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
                bbb_config.encoding_protocol = PLAIN;
                break;
            case 2:
                bbb_config.encoding_protocol = COBS_FRAME;
                break;
            default:
                fprintf(stderr, "Illegal option -e: %d\n", val);
                return 1;
            }
            break;
        }
        case 'u': {
            // store for later parsing
            printf("got a udaq config:[%s]\n", optarg);
            udaq_options[udaq_opt_count] = optarg;
            udaq_opt_count++;

            break;
        }
        case 'h':
            // clang-format off
            fprintf(stderr, "Usage: %s [-b buffer_size_bits] [-z page_size_bits] [-p port] [-e encoding] [-t trace] -u [udaq_specifier]\n", argv[0]);
            fprintf(stderr, "       %s [-h]\n", argv[0]);
            fprintf(stderr, "    -h  help                 Print usage\n");
            fprintf(stderr, "    -b  buffer_sz    Size, in bits, of the page buffer (DEFAULT=%d)\n", DEFAULT_BUFFER_SIZE);
            fprintf(stderr, "    -z  page_sz      Size, in bits, of a page buffer page (DEFAULT=%d)\n", DEFAULT_PAGE_SIZE);
            fprintf(stderr, "    -p  port                 sdaq-service: Port to run sdaq interface on (DEFAULT=%d\n", DEFAULT_BBB_COMMS_PORT);
  
            fprintf(stderr, "    -e  encoding             sdaq-service: Set the message encoding for the sdaq interface "
                            "(1=plain 2=cobs_frame DEFAULT=plain)\n");
            fprintf(stderr, "    -t  trace                sdaq-service: Set tracing level for the sdaq interface (0=none, 1=protocol, 2=stream, 3=both DEFAULT=none)\n");
            fprintf(stderr, "    -u  udaq_specifier       A quoted udaq configuration string:\n");

            fprintf(stderr, "\n");
            fprintf(stderr, "                            \"[-i ip_addr] [-p port] [-r hit_rate] [-g thread_group] [-e encoding] [-t trace]\"\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "                            Required:\n");
            fprintf(stderr, "                            -p  port             Udaq services comm port\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "                            Optional:\n");
            fprintf(stderr, "                            -i  ip_addr          udaq services ip address (DEFAULT=%s)\n", DEFAULT_UDAQ_IP);
            fprintf(stderr, "                            -g  thread_group     The thread group servicing the udaq (DEFAULT=0)\n");
            fprintf(stderr, "                            -e  encoding         Sets the message encoding (1=plain 2=cobs_frame DEFAULT=plain)\n");
            fprintf(stderr, "                            -t  trace            Sets tracing level (0=none, 1=protocol, 2=stream, 3=both DEFAULT=none)\n");

            fprintf(stderr, "\n");
            fprintf(stderr, "Examples:\n");
            fprintf(stderr, "%s -u \"-i 192.268.0.1 -p 6660\"\n",argv[0]);
            fprintf(stderr, "%s -u \"-i 192.268.0.1 -p 6660\" -u \"-i 192.268.0.1 -p 6661\"\n",argv[0]);
            fprintf(stderr, "%s -b 16 -z 10 -u \"-i 192.268.0.1 -p 6660 -g 0\" -u \"-i 192.268.0.1 -p 6661 -g 1\"\n",argv[0]);
            fprintf(stderr, "%s -p 7770 -e 1 -t 0 -u \"-i 192.268.0.1 -p 6660 -g 0 -e 2 -t 0\" -u \"-i 192.268.0.1 -p 6661 -g 1 -e 2 -t 0\"\n",argv[0]);
            fprintf(stderr, "\n");
            // clang-format on
            return 0;

        case '?':
            fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            return 1;
        default:
            abort();
        }

    // lightly abuse getopt() to parse the udaq options
    for (int i = 0; i < udaq_opt_count; i++) {
        optind = 1; // resets getopt() state
        if (!add_udaq_config(udaq_options[i])) {
            fprintf(stderr, "Bad udaq option [-u \'%s\'']\n", udaq_options[i]);
            return 1;
        }
    }

    print_config();

    // initialized bbb
    sim_bbb_t *bbb =
        sim_bbb_new(fh_connector_new_tcp_server(bbb_config.port), bbb_config.buffer_sz, bbb_config.page_sz);
    if (bbb_config.trace_protocol) {
        sim_bbb_trace_protocol(bbb, stdout);
    }
    if (bbb_config.trace_stream) {
        sim_bbb_trace_stream(bbb, stdout);
    }

    sim_bbb_start(bbb);

    // block waiting for bbb main loop to exit.
    sim_bbb_join(bbb);

    sim_bbb_destroy(&bbb);

    // clean up configs
    for (int i = 0; i < MAX_ATTACHED_UDAQS; i++) {
        if (bbb_config.udaq_conf[i]) {
            destroy_config(&(bbb_config.udaq_conf[i]));
        }
    }

    printf("\n");
    printf("XXXXXXXXXXXXXXXXXXXX\n");
    printf("XXXXXXXXXXXXXXXXXXXX\n");
    printf("XXXXXXXXXXXXXXXXXXXX\n");
    printf("XXXXXXXXXXXXXXXXXXXX\n");
    printf("main thread exit\n");
    printf("XXXXXXXXXXXXXXXXXXXX\n");
    printf("XXXXXXXXXXXXXXXXXXXX\n");
    printf("XXXXXXXXXXXXXXXXXXXX\n");
    printf("XXXXXXXXXXXXXXXXXXXX\n");
    printf("\n");
}

void
print_config()
{
    printf("launching field hub:\n");
    printf("    page-buffer:  page = %d, size = %d\n", (1 << bbb_config.page_sz), (1 << bbb_config.buffer_sz));
    printf("    sdaq-service: port = %d\n", bbb_config.port);
    printf("    sdaq-service: protocol = %d\n", bbb_config.encoding_protocol);
    printf("    sdaq-service: trace_protocol = %s\n", bbb_config.trace_protocol ? "true" : "false");
    printf("    sdaq-service: trace_stream = %s\n", bbb_config.trace_stream ? "true" : "false");
    printf("\n");

    if (bbb_config.num_config_udaqs < 1) {
        printf("    NOTE: No udaq configured.\n");
    }
    for (int i = 0; i < bbb_config.num_config_udaqs; i++) {
        udag_config_t *udaq = bbb_config.udaq_conf[i];
        printf("    udaq[%d] %s:%d\n", i, udaq->ip, udaq->port);
        printf("        thread-group = %d\n", udaq->thread_group);
        printf("        protocol = %d\n", udaq->encoding_protocol);
        printf("        trace_protocol = %s\n", udaq->trace_protocol ? "true" : "false");
        printf("        trace_stream = %s\n", udaq->trace_stream ? "true" : "false");
        printf("\n");
    }
}

bool
add_udaq_config(char *config_str)
{
#define MAX_SUBOPTS 5

    char copy[strlen(config_str) + 1];
    strcpy(copy, config_str);

    char *ip = DEFAULT_UDAQ_IP;
    uint16_t port = 0;
    uint8_t thread_group = 0;
    protocol encoding_protocol = DEFAULT_PROTOCOL_ENCODING;
    bool trace_protocol = false;
    bool trace_stream = false;

    // gather subargs
    int MAX_TOKENS = MAX_SUBOPTS * 2 + 1;
    char *subargs[MAX_TOKENS + 1];
    subargs[0] = "dummy"; // fake the executable name. getopt() expects this.
    int i = 1;
    char *tok;
    tok = strtok(copy, " ");
    while (tok != NULL) {
        if (i > MAX_TOKENS) {
            fprintf(stderr, "Too many udaq options in: [%s]\n", config_str);
            return false;
        }
        subargs[i] = tok;
        tok = strtok(NULL, " ");
        i++;
    }

    // process subargs with getopt
    int opt;
    while ((opt = getopt(i, subargs, "i:p:g:e:t:")) != -1) {

        switch (opt) {
        case 'i':
            ip = optarg;
            break;

        case 'p':
            port = atoi(optarg);
            break;
        case 'g':
            thread_group = atoi(optarg);
            break;

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
                return false;
            }
            break;
        }
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
                return false;
            }
            break;
        }

        case '?':
            fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            return false;
        default:
            abort();
        }
    }

    // port required
    if (port == 0) {
        fprintf(stderr, "Illegal option: Must enter port.\n");
        return false;
    }

    // add udaq config to global config
    bbb_config.udaq_conf[bbb_config.num_config_udaqs] =
        make_config(ip, port, thread_group, encoding_protocol, trace_protocol, trace_stream);
    bbb_config.num_config_udaqs++;

    return true;
}

// a shim for TBD configuration logic.  A production version
// would interrogate the serial devices looking for udaqs, look
// to a config file, or a combination thereof.
//
// this version sets up tcp/ip connections to simulated udaqs via
// the global bbb_config structure loaded from main.
//__attribute__((unused))
void
configure_udaqs(sim_bbb_t *bbb)
{
    for (int i = 0; i < MAX_ATTACHED_UDAQS; i++) {
        udag_config_t *conf = bbb_config.udaq_conf[i];
        if (conf) {

            // set up peer
            fh_connector_t *connector = fh_connector_new_tcp_client(conf->ip, conf->port);
            udaq_peer_t *peer = udaq_peer_new(i, connector);
            if (conf->trace_protocol) {
                udaq_peer_trace_protocol(peer, stdout);
            }

            // apply to bbb
            sim_bbb_configure_udaq(bbb, peer, conf->thread_group);
        }
    }
}
