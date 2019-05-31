/*
 * Test program for ASCII-to-binary message translation
 */

#include <stdio.h>
#include <string.h>

#include "fh_library.h"
//#include "udaq_api/udaq_comms_api.h"

int main(int argv, char **argc) {
    int i, len;
    char str[1024];
    extern msg_dict_t msg_dict;

    if (argv <= 1) {
        printf("Usage: %s <command> [arg1 arg2 ...]\n", argc[0]);
        return 0;
    }

    // Treat all command-line arguments as one string
    len = 0;
    for (i=1; i<argv; i++) {
        sprintf(&str[len], "%s ", argc[i]);
        len += strlen(argc[i])+1;
    }

    // Wrap ASCII input in message
    fh_message_t *msgin = fh_message_new();
    fh_message_t *msgout = fh_message_new();
    fh_message_init_ascii_msg(msgin, str);

    // Translate to binary
    if (fh_msg_translate(&msg_dict, msgin, msgout) == 0)
        fh_message_hexdump(msgout, argc[1], stdout);
    else
        printf("Error: couldn't convert command %s!\n", argc[1]);

    fh_message_destroy(&msgin);
    fh_message_destroy(&msgout);
    return 0;

}
