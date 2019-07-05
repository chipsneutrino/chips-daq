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



// function type for message translators
typedef enum {
    FH_ASCII_FORMAT,
    FH_BINARY_FORMAT
} fh_format;
typedef bool (*translate_fn)(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len);

// ################################
// bidirectional translators
// ################################
// empty data (enforced)
bool empty_input_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len);
#define XLAT_EMPTY &empty_input_translator

// passes the message content as is
bool asis_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len);
#define XLAT_AS_IS &asis_translator

// Generates a "?NO ASCII COMMAND TRANSLATION" error
bool no_ascii_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len);
#define XLAT_NONE &no_ascii_translator

// ################################
// standard input translators
// ################################

// translates space-seperated ascii tokens using pack format string
bool pack_input_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len);
#define XLAT_PACK &pack_input_translator

// ################################
// standard output translators
// ################################

// translates space-seperated ascii tokens using pack format string
bool pack_output_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len);
#define XLAT_UNPACK &pack_output_translator

// translates binary output message as hexdump
bool hex_output_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len);
#define XLAT_HEXDUMP &hex_output_translator

// holds mapping from ascii command string to binary [mt, mst]
// and translation definitions for inbound/outbound messages
typedef struct {
    char *str;              // the ASCI for of the command
    uint8_t mt;             // the binary message type
    uint8_t mst;            // the binary message subtype
    translate_fn ascii2bin; // performs the inbound translation
    void *ascii2bin_ctx;
    translate_fn bin2ascii; // performs the outbound translation
    void *bin2ascii_ctx;
} msg_dict_entry_t;

typedef struct {
    int len;
    msg_dict_entry_t *e;
} msg_dict_t;


typedef struct _fh_msg_translator_t fh_msg_translator_t;

// create a new translator
fh_msg_translator_t * fh_msg_translator_new(msg_dict_t *msg_dict);

// destroy a translator
void fh_msg_translator_destroy(fh_msg_translator_t **self_p);

// Translate from ASCII formated messages to binary messages
bool fh_msg_translate_ascii2bin(fh_msg_translator_t *self, fh_message_t *asciimsg, fh_message_t *binmsg, char *err_buf, size_t err_buf_len);

// Translate from binary messages to ASCII formatted messages
bool fh_msg_translate_bin2ascii(fh_msg_translator_t *self, fh_message_t *binmsg, fh_message_t *asciimsg, char *err_buf, size_t err_buf_len);


//################################################################################
// internal type: fh_translator_map_t
//
// Implements a lookup map to index translation definitions by bothe ascii and
// binary keys.
//################################################################################
typedef struct _fh_translator_map_t fh_translator_map_t;

// Create a new translator map
fh_translator_map_t* fh_translator_map_new(msg_dict_t *msg_dict);

// destroy a translator map
void fh_translator_map_destroy(fh_translator_map_t **self_p);

// lookup an entry by ascii key
msg_dict_entry_t* lookup_by_ascii(fh_translator_map_t *self, char *ascii_cmd);

// lookup an entry by binary key
msg_dict_entry_t* lookup_by_bin(fh_translator_map_t *self, uint8_t mt, uint8_t mst);


#endif
