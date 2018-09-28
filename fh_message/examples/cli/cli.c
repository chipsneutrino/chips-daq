/*
 * Simple program to exercise command-line interface protocol
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "fh_library.h"

int main(int argv, char **argc) {

    char prompt[] = "> ";

    //  Create a new CLI transport
    fh_transport_t *transport = fh_transport_new_cli(stdin, stdout);

    fh_message_t *msgin = fh_message_new();
    fh_message_t *msgout = fh_message_new();
    fh_message_init_ascii_msg(msgout, "OK\n");

    while (!feof(stdin)) {
        fputs(prompt, stdout);
        fflush(stdout);
        fh_transport_receive(transport, msgin);
        fh_message_hexdump(msgin, "", stdout);
        fh_transport_send(transport, msgout);
    }

    // Clean up
    fh_message_destroy(&msgin);
    fh_message_destroy(&msgout);
    fh_transport_destroy(&transport);

    return 0;

}
