
/**
 * sdaq.c
 *
 * Simulates an sdaq.
 */
#include <arpa/inet.h>
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// fh_message_lib : messaging library
#include "fh_library.h"

// fh_example_lib : shared example code
#include "cobs_frame_protocol.h"

// local
#include "fh_peer.h"
#include "sim_sdaq.h"

// bbb
#include "bbb_comms_api.h"

// forwards
void print_config();
bool add_fh_config(char *config_str);
static void _run_sequence(sim_sdaq_t *self);

// options
typedef enum { PLAIN, COBS_FRAME } protocol;

// defaults
#define DEFAULT_BBB_IP "127.0.0.1"
#define DEFAULT_PROTOCOL_ENCODING PLAIN

// holds config data for an attached field hub
typedef struct {
    char *ip;
    uint16_t port;
    protocol encoding_protocol; // message encoding scheme
    bool trace_protocol;        // trace messages on stdout
    bool trace_stream;          // trace stream operations on stdout
} fh_config_t;

fh_config_t *
new_fh_config(char *ip, uint16_t port, protocol encoding_protocol, bool trace_protocol, bool trace_stream)
{
    fh_config_t *self = (fh_config_t *)calloc(1, sizeof(fh_config_t));
    assert(self);

    size_t ip_len = strlen(ip);
    self->ip = calloc(1, ip_len);
    strncpy(self->ip, ip, ip_len);

    self->port = port;

    self->encoding_protocol = encoding_protocol;
    self->trace_protocol = trace_protocol;
    self->trace_stream = trace_stream;

    return self;
}

void
destroy_config(fh_config_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_config_t *self = *self_p;
        free(self->ip);
        free(self);
        *self_p = NULL;
    }
}

// holds config data for the run
#define MAX_ATTACHED_FIELD_HUBS 10
typedef struct {
    fh_config_t *fh_conf[MAX_ATTACHED_FIELD_HUBS]; // pointer to field hub config structures
    size_t num_fh_config;                          // number of configured field hubs
    uint32_t run_len_sec;                          // run length
} config;

config sdaq_config;

#define DEFUALT_RUN_DURATION 0xffffffff
int
main(int argc, char *argv[])
{

    sdaq_config.run_len_sec = DEFUALT_RUN_DURATION;

    int opt;
    char *fh_options[MAX_ATTACHED_FIELD_HUBS];
    int fh_opt_count = 0;
    while ((opt = getopt(argc, argv, "hd:f:")) != -1)
        switch (opt) {
            break;

        case 'd': {
            sdaq_config.run_len_sec = atoi(optarg);
            break;
        }

        case 'f': {
            // store for later parsing
            fh_options[fh_opt_count] = optarg;
            fh_opt_count++;

            break;
        }
        case 'h':
            // clang-format off
            fprintf(stderr, "Usage: %s [-d run_duration] [-f \"field_hub_spec\"]...\n", argv[0]);
            fprintf(stderr, "Usage: %s [-h]\n", argv[0]);
            fprintf(stderr, "\n");
            fprintf(stderr, "    -d  run_duration         Run duration in seconds (DEFAULT=%u)\n", DEFUALT_RUN_DURATION);
            fprintf(stderr, "    -f  field_hub_spec       A quoted config string for a field hub connection:\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "                            \"[-i ip_addr] [-p port] [-e encoding] [-t trace]\"\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "                            Optional:\n");
            fprintf(stderr, "                            -i  ip_addr      FieldHub services comm port (DEFAULT=%s)\n", DEFAULT_BBB_IP);
            fprintf(stderr, "                            -p  port         FieldHub services comm port (DEFAULT=%d)\n", DEFAULT_BBB_COMMS_PORT);
            fprintf(stderr, "                            -e  encoding     Sets the message encoding (msg_encoding 1=plain 2=cobs_frame DEFAULT=plain)\n");
            fprintf(stderr, "                            -t  trace        Sets tracing level (trace_option 0=none 1=protocol 2=stream 3=both DEFAULT=none)\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "                            Examples:\n");
            fprintf(stderr, "                            %s -d 120 -f \"-i 127.0.0.1 -p 7770\"\n", argv[0]);
            fprintf(stderr, "                            %s -d 120 -f \"-i 127.0.0.1 -p 7770 -e 1 -t 0\" -f \"-i 127.0.0.1 -p 7771 -e 1 -t 0\"\n", argv[0]);
            fprintf(stderr, "\n");
            fprintf(stderr, "    -h  help                Print usage\n");
            // clang-format on
            return 0;
        case '?':
            fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            return 1;
        default:
            abort();
        }

    // lightly abuse getopt() to parse the field hub options
    for (int i = 0; i < fh_opt_count; i++) {
        optind = 1; // resets getopt() state
        if (!add_fh_config(fh_options[i])) {
            fprintf(stderr, "Bad field hub option [-f \'%s\'']\n", fh_options[i]);
            return 1;
        }
    }

    print_config();

    // initialized sdaq
    sim_sdaq_t *sdaq = sim_sdaq_new();
    sim_sdaq_start(sdaq);

    // drive a run sequence
    _run_sequence(sdaq);

    // block waiting for sdaq main loop to exit.
    sim_sdaq_join(sdaq);

    // clean up configs
    for (int i = 0; i < MAX_ATTACHED_FIELD_HUBS; i++) {
        if (sdaq_config.fh_conf[i]) {
            destroy_config(&(sdaq_config.fh_conf[i]));
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

    printf("sdaq exiting\n");
}

void
print_config()
{
    printf("launching sdaq:\n");

    if (sdaq_config.num_fh_config < 1) {
        printf("    NOTE: No field hubs configured.\n");
    }
    for (int i = 0; i < sdaq_config.num_fh_config; i++) {
        fh_config_t *hub = sdaq_config.fh_conf[i];
        printf("    fieldhub[%d] %s:%d\n", i, hub->ip, hub->port);
        printf("        protocol = %d\n", hub->encoding_protocol);
        printf("        trace_protocol = %s\n", hub->trace_protocol ? "true" : "false");
        printf("        trace_stream = %s\n", hub->trace_stream ? "true" : "false");
        printf("\n");
    }
}

bool
add_fh_config(char *config_str)
{
#define MAX_SUBOPTS 5

    char copy[strlen(config_str) + 1];
    strcpy(copy, config_str);

    char *ip = DEFAULT_BBB_IP;
    uint16_t port = DEFAULT_BBB_COMMS_PORT;
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
    while ((opt = getopt(i, subargs, "i:p:e:t:")) != -1) {

        switch (opt) {
        case 'i':
            ip = optarg;
            break;

        case 'p':
            port = atoi(optarg);
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

    // ip, port, thread group are required
    // if ((ip == NULL) || (port == 0)) {
    //     fprintf(stderr, "Illegal option: Must enter ip, port.\n");
    //     return false;
    // }

    // add udaq config to global config
    sdaq_config.fh_conf[sdaq_config.num_fh_config] =
        new_fh_config(ip, port, encoding_protocol, trace_protocol, trace_stream);
    sdaq_config.num_fh_config++;

    return true;
}

// a shim for TBD configuration logic. A production implementation
// would learn about its attached field hubs at configutration time, possibly
// with information being provided by a higher leve system at that time.
void
configure_field_hubs(sim_sdaq_t *sdaq)
{
    for (int i = 0; i < sdaq_config.num_fh_config; i++) {
        fh_config_t *conf = sdaq_config.fh_conf[i];
        if (conf) {

            // set up peer
            fh_connector_t *connector = fh_connector_new_tcp_client(conf->ip, conf->port);
            fh_peer_t *peer = fh_peer_new(i, connector);
            if (conf->trace_protocol) {
                fh_peer_trace_protocol(peer, stdout);
            }

            // apply to sdaq
            sim_sdaq_configure_fh(sdaq, peer);
        }
        else {
            printf("bad field hub conf at %d\n", i);
            abort();
        }
    }
}

// run sequence.
// In a production version, this sequnce may be controlled by a remote component
// via another set of control messages.
static void
_run_sequence(sim_sdaq_t *self)
{
    printf("CONFIGURING RUN...\n");
    sim_sdaq_configure(self);

    sleep(2);

    printf("STARTING RUN...\n");
    sim_sdaq_start_run(self);

    sleep(sdaq_config.run_len_sec);

    printf("STOPPING RUN...\n");
    sim_sdaq_stop_run(self);
}
