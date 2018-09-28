/**
 * udaq.c
 *
 * Front end application for running udaq device simulations.
 * Supports a udaq-per-process use model, or a many-udaq-per-process
 * model. 
 */
#include "sim_udaq.h"
#include "standard_inc.h"

#include "fh_library.h"


typedef enum { PLAIN, COBS_FRAME } msg_protocol;

#define MAX_UDAQS 32 // max number of simulated udaqs

// defaults for simulated udaq instances
#define DEFAULT_UDAQ_PORT 6660
#define DEFAULT_BUFFER_SIZE 16
#define DEFAULT_PAGE_SIZE 10
#define DEFAULT_HIT_RATE 100
#define DEFAULT_MSG_ENCODING PLAIN
#define DEFAULT_P_TRACE false;
#define DEFAULT_S_TRACE false

// holds config parameters for a sim_udaq_t
typedef struct {
    uint64_t device_id;    // device id
    uint8_t buffer_sz;     // buffer size (bits)
    uint8_t page_sz;       // buffer page size (bits)
    uint32_t hit_rate;     // hit rate (Hz)
    uint16_t port;         // udaq service port
    msg_protocol encoding; // message encoding scheme
    bool trace_protocol;   // trace messages on stdout
    bool trace_stream;     // trace stream operations on stdout
} sim_udaq_conf_t;

// holds overall config for simulation
typedef struct {
    sim_udaq_conf_t *udaq_conf_v[MAX_UDAQS]; // pointers to sim_conf_t instances
    sim_udaq_t *udaq_v[MAX_UDAQS];           // pointers to sim_udaq_t instances
    size_t num_udaqs;                        // number of sim udaqs
} main_conf_t;

// forwards
sim_udaq_conf_t *
parse_udaq_config(char *config_str);
sim_udaq_t *
make_udaq(sim_udaq_conf_t *udaq_conf);
void
print_config(main_conf_t *config);

int
main(int argc, char *argv[])
{

    main_conf_t config;

    // parse command line:
    // -u "[udaq specifier]" -u "[udaq specifier]" -u "[udaq specifier]"
    int opt;
    char *udaq_spec_string[MAX_UDAQS];
    int udaq_spec_count = 0;
    while ((opt = getopt(argc, argv, "hu:")) != -1)
        switch (opt) {
        case 'u':
            // store for later parsing
            udaq_spec_string[udaq_spec_count] = optarg;
            udaq_spec_count++;

            break;
        case 'h':
            // clang-format off
            fprintf(stderr, "Usage: %s [-u \"udaq_specifier\"] ...\n", argv[0]);
            fprintf(stderr, "       %s [-h]\n", argv[0]);
            fprintf(stderr, "\n");
            fprintf(stderr, "    -u  udaq_specifier      A quoted udaq configuration string:\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "                            \"[-d dev_id] [-p port] [-r hit_rate] [-b buffer_size_bits] [-z page_size_bits]\n");
            fprintf(stderr, "                             [-e msg_encoding 1=plain 2=cobs_frame] [-t trace_option 0=none, 1=protocol,2=stream, 3=both]\"\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "                            Required:\n");
            fprintf(stderr, "                            -d  dev_id       The device id as a 64-bit number in hexidecimal format\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "                            Optional:\n");
            fprintf(stderr, "                            -p  port         Udaq services comm port (DEFAULT=%d)\n", DEFAULT_UDAQ_PORT);
            fprintf(stderr, "                            -r  hit_rate     The simulated hit rate (DEFAULT=%d)\n", DEFAULT_HIT_RATE);
            fprintf(stderr, "                            -b  buffer_sz    Size, in bits, of the hit buffer (DEFAULT=%d)\n", DEFAULT_BUFFER_SIZE);
            fprintf(stderr, "                            -z  page_sz      Size, in bits, of a hit buffer page (DEFAULT=%d)\n", DEFAULT_PAGE_SIZE);
            fprintf(stderr, "                            -e  encoding     Sets the message encoding (DEFAULT=plain)\n");
            fprintf(stderr, "                            -t  trace        Sets tracing level (DEFAULT=none)\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "                            Examples:\n");
            fprintf(stderr, "                            %s -u \"-d 0x12345678abcdef01\"\n", argv[0]);
            fprintf(stderr, "                            %s -u \"-d 0x01 -p 6660\" -u \"-d 0x02 -p 6661\" -u \"-d 0x02 -p 6662\"\n", argv[0]);
            fprintf(stderr, "                            %s -u \"-d 0x01 -p 6660 -e 2\" - u \"-d 0x02 -p 6661 -e 2\"\n", argv[0]);
            fprintf(stderr, "                            %s -u \"-d 0x01 -p 6660 -e 2 -r 10 -b 19 -z 8\"\n", argv[0]);
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

    // lightly abuse getopt() to parse the udaq spec strings into sim_udaq_conf_t instances
    for (int i = 0; i < udaq_spec_count; i++) {
        optind = 1; // resets getopt() state

        config.udaq_conf_v[i] = parse_udaq_config(udaq_spec_string[i]);
        if (!config.udaq_conf_v[i]) {
            fprintf(stderr, "Bad udaq option [-u \'%s\'']\n", udaq_spec_string[i]);
            return 1;
        }
        else {
            config.num_udaqs++;
        }
    }

    print_config(&config);


    // instantiate the udaqs
    for (int i = 0; i < config.num_udaqs; i++) {

        config.udaq_v[i] = make_udaq(config.udaq_conf_v[i]);
    }

    // start up the udaqs
    for (int i = 0; i < config.num_udaqs; i++) {

        sim_udaq_start(config.udaq_v[i]);
    }

    // wiat for udaqs to complete
    for (int i = 0; i < config.num_udaqs; i++) {

        sim_udaq_join(config.udaq_v[i]);
    }

    // cleanup
    for (int i = 0; i < config.num_udaqs; i++) {

        sim_udaq_destroy(&(config.udaq_v[i]));
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

// parse a udaq config string into a newly allocated sim_udaq_conf_t
// -d [device_id] -p [port] -r [hit-rate] -b [buffer_size] - z [page_size] -e [msg_encoding] -t [trace_option]
sim_udaq_conf_t *
parse_udaq_config(char *config_str)
{
#define MAX_SUBOPTS 5

    char copy[strlen(config_str) + 1];
    strcpy(copy, config_str);

    uint64_t device_id = 0;
    uint8_t buffer_sz = DEFAULT_BUFFER_SIZE;
    uint8_t page_sz = DEFAULT_PAGE_SIZE;
    uint32_t hit_rate = DEFAULT_HIT_RATE;
    uint16_t port = DEFAULT_UDAQ_PORT;
    msg_protocol encoding = DEFAULT_MSG_ENCODING;
    bool trace_protocol = DEFAULT_P_TRACE;
    bool trace_stream = DEFAULT_S_TRACE;

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
            return NULL;
        }
        subargs[i] = tok;
        tok = strtok(NULL, " ");
        i++;
    }

    // for (int j = 0; j < i; j++) {
    //     printf("subarg[%d] = [%s]\n", j, subargs[j]);
    // }

    // process subargs with getopt
    // -d [device_id] -p [port] -r [hit-rate] -b [buffer_size] - z [page_size] -e [msg encoding] -t [trace_option]
    int opt;
    while ((opt = getopt(i, subargs, "d:p:r:b:z:e:t:")) != -1) {

        switch (opt) {
        case 'd':
            // sscanf(optarg, "%016llX", &device_id);
            device_id = strtoull(optarg, NULL, 16);
            break;

        case 'p':
            port = atoi(optarg);
            break;
        case 'r':
            hit_rate = atoi(optarg);
            break;
        case 'b':
            buffer_sz = atoi(optarg);
            break;
        case 'z':
            page_sz = atoi(optarg);
            break;

        case 'e': {
            int val = atoi(optarg);
            switch (val) {
            case 1:
                encoding = PLAIN;
                break;
            case 2:
                encoding = COBS_FRAME;
                break;
            default:
                fprintf(stderr, "Illegal option -e: %d\n", val);
                return NULL;
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
            return NULL;
        default:
            abort();
        }
    }

    // device_id required
    if (device_id == 0) {
        fprintf(stderr, "Illegal option: Must enter a device_id.\n");
        return NULL;
    }

    sim_udaq_conf_t *udaq_conf = (sim_udaq_conf_t *)calloc(1, sizeof(sim_udaq_conf_t));
    assert(udaq_conf);
    udaq_conf->device_id = device_id;
    udaq_conf->buffer_sz = buffer_sz;
    udaq_conf->page_sz = page_sz;
    udaq_conf->hit_rate = hit_rate;
    udaq_conf->port = port;
    udaq_conf->encoding = encoding;
    udaq_conf->trace_protocol = trace_protocol;
    udaq_conf->trace_stream = trace_stream;

    return udaq_conf;
}

// make a sim_udaq_t from a sim_udaq_config_t
sim_udaq_t *
make_udaq(sim_udaq_conf_t *udaq_conf)
{
    sim_udaq_t *udaq = sim_udaq_new((sim_udaq_spec){.device_id = udaq_conf->device_id,
                                                    .buffer_sz = udaq_conf->buffer_sz,
                                                    .page_sz = udaq_conf->page_sz,
                                                    .hit_rate = udaq_conf->hit_rate},
                                    fh_connector_new_tcp_server(udaq_conf->port));

    // todo: apply msg encoding : encoding
    // todo: apply tracing:  trace_protocol, trace_stream

    return udaq;
}

void
print_config(main_conf_t *config)
{
    printf("launching udaq simulators:\n");
    printf("\n");

    if (config->num_udaqs < 1) {
        printf("    NOTE: No udaqs configured.\n");
    }
    else {
        for (int i = 0; i < config->num_udaqs; i++) {
            sim_udaq_conf_t *udaq = config->udaq_conf_v[i];
            printf("    sim-udaq[%d] port:%d\n", i, udaq->port);
            printf("        page-buffer: page = %d, size = %d\n", (1 << udaq->page_sz), (1 << udaq->buffer_sz));
            printf("        device_id = 0x%016llx\n", udaq->device_id);
            printf("        encoding = %d\n", udaq->encoding);
            printf("        trace_protocol = %s\n", udaq->trace_protocol ? "true" : "false");
            printf("        trace_stream = %s\n", udaq->trace_stream ? "true" : "false");
            printf("\n");
        }
    }
}
