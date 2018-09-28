/**
 * fh_transport_test.c
 *
 * Tests a message transport implementation by exchanging messages with
 * payloads data generated from a pseudorandom byte sequence between a
 * client and server.
 *
 * Usage:  
 * 
 *  1. Initialize the server side of the transport and call _server_th().
 *  2. Initialize the client side of the transport and call _client_th().
 */

#include "../fh_classes.h"

#include <stdarg.h>

// controls message payload size range
#define MIN_PAYLOAD_SIZE 1
#define MAX_PAYLOAD_SIZE 100
// #define MAX_PAYLOAD_SIZE 4090
// #define MAX_PAYLOAD_SIZE 65329

//Message op codes
#define TRANSPORT_TEST_OPCODE 27
#define PRBS_PAYLOAD_SUBCODE 1            // pseudo-random payload issues by client
                                          // and verified by server

// client issues
// [27][3]<byte-sequence>
//
// server verifies byte sequence and replies with
// [27][3]<status 1=pass, 2=fail>


//forwards
static uint32_t prbs(void);
static void logtrace(FILE *dst, char *str, ...);


// Entry point for the server
void *
fh_transport_test_server_th(fh_transport_t *transport, FILE *log)
{
    fh_message_t *msg = fh_message_new();

    uint32_t msgnum = 0;

    while (true) {
        msgnum++;

        logtrace(log, "#################################################################################\n");
        logtrace(log, "Server reading message %ld ...\n", msgnum);
        logtrace(log, "#################################################################################\n");
        int status = fh_transport_receive(transport, msg);

        if (status == 0) {
            int len = fh_message_dataLen(msg);
            bool intact = true;
            uint8_t *data = fh_message_getData(msg);
            for (int i = 0; i < len; i++) {
                uint8_t datum = prbs() & 0xFF;
                bool match = (data[i] == datum);
                if (!match) {
                    logtrace(log, "mismatch at byte %d expected %#x got %#x\n", i, datum, data[i]);
                }
                intact = intact && match;
            }
            logtrace(log, "SERVER received message %lu, length: %d, status: %s.\n", msgnum, len,
                     (intact ? "OK" : "CORRUPT"));

            logtrace(log, "\n\n");

            uint8_t respcode = intact ? 1 : 2;
            fh_message_init_full(msg, TRANSPORT_TEST_OPCODE, PRBS_PAYLOAD_SUBCODE, &respcode, 1);
            logtrace(log, "SERVER sending response status %d...\n", respcode);
            // fflush(stdout);
            status = fh_transport_send(transport, msg);
            logtrace(log, "SERVER sent response\n");

            if (status != 0) {
                logtrace(log, "SERVER response send() failed:  %d\n", status);
                abort();
            }

            if (!intact) {
                break;
            }

            logtrace(log, "\n\n");
            // fflush(stdout);
        }
        else {
            logtrace(log, "SERVER receive() failed:  %d\n", status);
            abort();
        }
    }

    logtrace(log, "SERVER exit()\n");

    return NULL;
}


// Entry point for the client
void *
fh_transport_test_client_th(fh_transport_t *transport, FILE *log)
{
    fh_message_t *msg = fh_message_new();
    uint32_t msgnum = 0;

    // random, but repeatable for debugging
    fh_rand_t *rand = fh_rand_new();

    uint64_t sent_bytes = 0;
    while (true) {
        msgnum++;

        // randomize message payload size
        uint32_t PAYLOAD_SIZE = fh_rand_int(rand, MIN_PAYLOAD_SIZE, MAX_PAYLOAD_SIZE);
        logtrace(log, "%ld msg size: %d\n", msgnum, PAYLOAD_SIZE);

        uint8_t msgdat[PAYLOAD_SIZE];
        for (int i = 0; i < PAYLOAD_SIZE; i++) {
            msgdat[i] = prbs() & 0xFF;
        };
        fh_message_init_full(msg, TRANSPORT_TEST_OPCODE, PRBS_PAYLOAD_SUBCODE, msgdat, PAYLOAD_SIZE);

        // Checking the message length resolves issues
        // of message initialization truncating payloads that dont fit in the allocated
        // message buffer
        if (fh_message_dataLen(msg) != PAYLOAD_SIZE) {
            printf("WARN: Payload was truncated ... test should fail\n");
        }

        logtrace(log, "#################################################################################\n");
        logtrace(log, "CLIENT sending message %lu, payload size: %d ...\n", msgnum, PAYLOAD_SIZE);
        logtrace(log, "#################################################################################\n");
        int status = fh_transport_send(transport, msg);

        if (status == 0) {
            sent_bytes += PAYLOAD_SIZE;
            logtrace(log, "CLIENT sent message %lu, payload size: %d ...\n", msgnum, PAYLOAD_SIZE);
            logtrace(log, "Sent Bytes: %llu\n", sent_bytes);
        }
        else {
            logtrace(log, "CLIENT send() failed:  %d\n", status);
            abort();
        }

        logtrace(log, "\n\n");

        // get the servers response
        logtrace(log, "CLIENT reading server response\n");
        status = fh_transport_receive(transport, msg);

        if (status == 0) {
            uint8_t *resp = fh_message_getData(msg);
            if (*resp != 1) {
                logtrace(log, "SERVER reported bad message:  %d\n", status);
                break;
            }
            else {
                logtrace(log, "CLIENT read server response as: %d\n", *resp);
            }
        }
        else {
            logtrace(log, "CLIENT receive() failed:  %d\n", status);
            abort();
        }

        logtrace(log, "\n\n");
    }
    logtrace(log, "CLIENT exit()\n");

    return NULL;
}

#define PRBS_IV 0x1
/*
 * PRBS31 generator according to IEEE 802.3-2008 49.2.8
 * polynomial: G(x) = 1 + x^28 + x^31, output inverted
 */
static uint32_t
prbs(void)
{
    static uint32_t lfsr = PRBS_IV;
    lfsr = (lfsr >> 1) ^ (-(lfsr & (uint32_t)1) & (((uint32_t)1 << 30) | ((uint32_t)1 << 27)));
    return (~lfsr);
}

// trace logging function
static void
logtrace(FILE *dst, char *str, ...)
{
    if (dst) {
        va_list arg;
        va_start(arg, str);
        vfprintf(dst, str, arg);
        va_end(arg);
    }
    else {
        // to /dev/null!
    }
}
