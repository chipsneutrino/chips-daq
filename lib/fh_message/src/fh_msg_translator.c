/*
 * fh_msg_translator.c
 *
 * ASCII-to-binary message translator
 */

#include "fh_classes.h"

#include "ascii/fh_ascii_api.h"

#include "uthash/uthash.h"
#include "uthash/uthash.h"


/* Since strupr() is non-standard */
char* upper(char* s) {
    char* tmp = s;
    for (;*tmp;++tmp)
        *tmp = toupper((unsigned char) *tmp);    
    return s;
}

struct _fh_msg_translator_t {
    msg_dict_t *msg_dict;            // source of function-specific translators
    fh_translator_map_t *lookup;     // translators indexed by ascii and binary keys
};


// create a new translator
fh_msg_translator_t * fh_msg_translator_new(msg_dict_t *msg_dict)
{
    fh_msg_translator_t *self = (fh_msg_translator_t *)calloc(1, sizeof(fh_msg_translator_t));
    assert(self);
    self->msg_dict = msg_dict;
    self->lookup = fh_translator_map_new(msg_dict);
    
    return self;
}

// destroy a translator
void fh_msg_translator_destroy(fh_msg_translator_t **self_p)
{

    assert(self_p);
    if (*self_p) {
        fh_msg_translator_t *self = *self_p;

        // Note: client retains ownership of msg_dict
        //self->msg_dict;

        fh_translator_map_destroy(&(self->lookup));

        free(self);
    }

    *self_p = NULL;
}

// Translate from ASCII formated messages to binary messages
bool
fh_msg_translate_ascii2bin(fh_msg_translator_t *self, fh_message_t *asciimsg,
                           fh_message_t *binmsg, char *err_buf, size_t err_buf_len)
{
    const uint8_t *src = fh_message_getData(asciimsg);
    uint8_t *dst = fh_message_getData(binmsg);

    uint16_t src_len = fh_message_dataLen(asciimsg);
    uint16_t dst_len = fh_message_getMaxDataLen();


    //todo lookup command using space/null terminate string directly from
    //     message rather than a copy?
    size_t cmdlen = strcspn((char *)src, " "); // works for space or null terminates str
    char cmd[cmdlen+1];
    memcpy(cmd, src, cmdlen);
    cmd[cmdlen] = '\0';
    upper(cmd);

    // Search for the string in the message dictionary
     msg_dict_entry_t *entry = lookup_by_ascii(self->lookup, cmd);

    if(!entry)
    {
        //encode ascii error message into err_buf
        init_ascii_invalid_command_err(err_buf, err_buf_len, cmd);
        return false;
    }

    //initialize message header
    fh_message_init(binmsg, entry->mt, entry->mst);

    // advance source past ascii cmd
    int i = 0;
    do {
        i++;
    }  while (src[i] != ' ' && i < src_len);

    src += i;
    src_len -= i;

    // call the specific transformation function
    if(!entry->ascii2bin)
    {
        init_ascii_xlat_err(err_buf, err_buf_len, __FILE__, __LINE__);
        return false;
    }
    size_t used = 0;
    fh_format format;  // not used on inbound
    bool success = (*(entry->ascii2bin))(entry->ascii2bin_ctx, src, src_len, dst, dst_len, &used, &format, err_buf, err_buf_len);

    if(success)
    {
        fh_message_setDataLen(binmsg, used);
    }

    return success;
    
}

// Translate from binary messages to ASCII formatted messages
bool
fh_msg_translate_bin2ascii(fh_msg_translator_t *self, fh_message_t *binmsg, fh_message_t *asciimsg, char *err_buf, size_t err_buf_len)
{
    uint8_t mt = fh_message_getType(binmsg);
    uint8_t mst = fh_message_getSubtype(binmsg);

    const uint8_t *src = fh_message_getData(binmsg);
    uint8_t *dst = fh_message_getData(asciimsg);

    uint16_t src_len = fh_message_dataLen(binmsg);
    uint16_t dst_len = fh_message_getMaxDataLen();

    // translate system-defined error messages directly
    if (mt == ERR_SERVICE) {
        translate_binary_err(binmsg, err_buf, err_buf_len);
        fh_message_setData(asciimsg, (uint8_t*)err_buf, strnlen(err_buf, err_buf_len) + 1);
        return true;
    }

    //lookup tranlator
    msg_dict_entry_t *entry = lookup_by_bin(self->lookup, mt, mst);

    bool success;
    if (entry) {
        // call the specific transformation function
        if (!entry->bin2ascii) {
            init_ascii_xlat_err(err_buf, err_buf_len, __FILE__, __LINE__);
            return false;
        }
        size_t used = 0;
        fh_format format = FH_ASCII_FORMAT;
        success = (*(entry->bin2ascii))(entry->bin2ascii_ctx, src, src_len, dst, dst_len, &used, &format,  err_buf, err_buf_len);

        if (success) {
            switch (format) {
            case FH_ASCII_FORMAT:
                fh_message_setSubtype(asciimsg, ASCII_RESP);
                break;
            case FH_BINARY_FORMAT:
                fh_message_setSubtype(asciimsg, BINARY_RESP);
                break;
            default:
                init_ascii_xlat_err(err_buf, err_buf_len, __FILE__, __LINE__);
                return false;
                break;
            }
            fh_message_setDataLen(asciimsg, used);
        }
    }
    else {
        init_ascii_xlat_err(err_buf, err_buf_len, __FILE__, __LINE__);
        success = false;
    }

    return success;
}

// ######################################################################################
// Translators
// ######################################################################################

// empty data (enforced)
bool
empty_input_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt,
                     char *err_buf, size_t err_buf_len)
{
    if (src_len != 0) {
        init_ascii_invalid_count_err(err_buf, err_buf_len);
        return false;
    }
    else {
        *used = 0;
        return true;
    }
}

// passes the message content as is
bool
asis_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len)
{
    if(dst_len < src_len)
    {
        init_ascii_overlow_err(err_buf, err_buf_len, 0, dst_len, src_len);
        return false;
    }

    memcpy(dst, src, src_len);
    *used = src_len;

    *output_fmt = (fh_format)ctx;
    return true;
}

// passes the message content as is
bool
no_ascii_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len)
{
        init_ascii_no_translate_err(err_buf, err_buf_len);
        return false;
}

// pack based input translator
bool
pack_input_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len)
{

    const char *fmt = (const char*)ctx;

    // extract parameters from ascii payload and encode into binary payload
    bool success = fh_pack_convert_ascii2bin(src, src_len, dst, dst_len, used, fmt, err_buf, err_buf_len);

    return success;
}

// pack based output translator
bool
pack_output_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len)
{
    const char *fmt = (const char *)ctx;

    // extract parameters from binary message and encode into ascii message
    bool success = fh_pack_convert_bin2ascii(src, src_len, dst, dst_len, used, fmt,  err_buf, err_buf_len);

    *output_fmt = FH_ASCII_FORMAT;
    return success;
}

// output translator that encodes binary data as hexdump
bool
hex_output_translator(void *ctx, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used, fh_format *output_fmt, char *err_buf, size_t err_buf_len)
{

    int written = fh_util_hexdump_b(src, src_len, dst, dst_len);

    if(written > 0)
    {
        *used = written;
        *output_fmt = FH_ASCII_FORMAT;
        return true;
    }
    else
    {
       return false;
    }

}

//################################################################################
// internal type: fh_translator_map_t
//
// Implements a lookup map to index translation definitions by bothe ascii and
// binary keys.
//################################################################################

// struct to hash entries by [mt, mst]
typedef struct _bin_key_t bin_key_t;
struct _bin_key_t {
    uint8_t mt;  // binary type
    uint8_t mst; // binary subtype
};

// struct to hold entries in hashmaps
// (hh1 indexed by cmd, hh2 indexted by [mt, mst])
typedef struct _fh_entry_node_t fh_entry_node_t;
struct _fh_entry_node_t {
    msg_dict_entry_t *entry; // the indexed entry
    const char *cmd;         // ascii lookup key
    bin_key_t bin_key;       // binary lookup key
    UT_hash_handle hh1;      // required to support hashing by name
    UT_hash_handle hh2;      // required to support hashing by mt/mst
};

// holds entries as list and also
// mapped by ascii cmd and by [mt, mst]
struct _fh_translator_map_t {
    fh_entry_node_t *map_by_ascii;
    fh_entry_node_t *map_by_bin;
};

// create a new sort node
fh_entry_node_t *
fh_entry_node_new(msg_dict_entry_t *entry)
{
    fh_entry_node_t *self = (fh_entry_node_t *)calloc(1, sizeof(fh_entry_node_t));
    assert(self);
    self->entry = entry;
    self->cmd = entry->str;
    self->bin_key.mt = entry->mt;
    self->bin_key.mst = entry->mst;

    return self;
}

// destroy a sort node
void
fh_entry_node_destroy(fh_entry_node_t **self_p)
{

    assert(self_p);
    if (*self_p) {
        fh_entry_node_t *self = *self_p;

        // todo free entry? these are probably
        //     definesd as global constants

        free(self);
    }

    *self_p = NULL;
}

// Create a new translator map
fh_translator_map_t *
fh_translator_map_new(msg_dict_t *msg_dict)
{
    fh_translator_map_t *self = (fh_translator_map_t *)calloc(1, sizeof(fh_translator_map_t));
    assert(self);

    // hash the entry list
    for (int i = 0; i < msg_dict->len; i++) {
        fh_entry_node_t *entry = fh_entry_node_new(&(msg_dict->e[i]));

        // hash by ascii cmd
        HASH_ADD_KEYPTR(hh1, self->map_by_ascii, entry->cmd, strlen(entry->cmd), entry);

        // hash by mt/mst
        HASH_ADD(hh2, self->map_by_bin, bin_key, sizeof(bin_key_t), entry);
    }
    return self;
}

// destroy a translator map
void
fh_translator_map_destroy(fh_translator_map_t **self_p)
{

    assert(self_p);
    if (*self_p) {
        fh_translator_map_t *self = *self_p;

        // HASH_CLEAR(hh1, self->map_by_ascii);
        HASH_CLEAR(hh2, self->map_by_bin);

        // use the ascii hash to delete the nodes
        fh_entry_node_t *cur_node, *tmp;
        HASH_ITER(hh1, self->map_by_ascii, cur_node, tmp)
        {
            HASH_DELETE(hh1, self->map_by_ascii, cur_node);
            fh_entry_node_destroy(&cur_node);
        }

        free(self);
    }

    *self_p = NULL;
}

// lookup an entry by ascii key
msg_dict_entry_t *
lookup_by_ascii(fh_translator_map_t *self, char *cmd)
{
    fh_entry_node_t *node = NULL;
    HASH_FIND(hh1, self->map_by_ascii, cmd, strlen(cmd), node);

    if (node) {
        return node->entry;
    }
    else {
        return NULL;
    }
}

// lookup an entry by binary key
msg_dict_entry_t *
lookup_by_bin(fh_translator_map_t *self, uint8_t mt, uint8_t mst)
{
    fh_entry_node_t *node = NULL;
    bin_key_t bin_key = {.mt = mt, .mst = mst};
    HASH_FIND(hh2, self->map_by_bin, &bin_key, sizeof(bin_key_t), node);

    if (node) {
        return node->entry;
    }
    else {
        return NULL;
    }
}
