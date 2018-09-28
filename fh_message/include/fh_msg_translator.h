/*
 * fh_msg_translator.h
 *
 * ASCII-to-binary message translator
 */

#ifndef FH_MSG_TRANSLATOR_H_INCLUDED
#define FH_MSG_TRANSLATOR_H_INCLUDED

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "fh_library.h"

/*-------------------------------------------------------------------*/
/* Mapping from string to message type and subtype */
typedef struct {
    char *str;
    uint8_t mt;
    uint8_t mst;
} msg_dict_entry_t;

typedef struct {
    int len;
    msg_dict_entry_t *e;
} msg_dict_t;

/* Maximum number of int32_t arguments from ASCII command */
#define MAX_COMMAND_ARGS 5

/* Maximum size of translated payload, in bytes */
#define MAX_COMMAND_SIZE (MAX_COMMAND_ARGS*4)

int fh_msg_translate(msg_dict_t *msg_dict, fh_message_t *msgin, fh_message_t *msgout);

#endif
