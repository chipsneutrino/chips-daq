/*
 * fh_msg_translator.c
 *
 * ASCII-to-binary message translator
 */

#include "fh_msg_translator.h"

/*-------------------------------------------------------------------*/

/* Since strupr() is non-standard */
char* upper(char* s) {
    char* tmp = s;
    for (;*tmp;++tmp)
        *tmp = toupper((unsigned char) *tmp);    
    return s;
}

/* Dict string comparison for search */
int msg_dict_compare(const void *s1, const void *s2) {
    const msg_dict_entry_t *e1 = s1;
    const msg_dict_entry_t *e2 = s2;
    return strcmp(e1->str, e2->str);
}

/*-------------------------------------------------------------------*/
/* Translate from ASCII command strings to binary messages */
int fh_msg_translate(msg_dict_t *msg_dict, fh_message_t *msgin, fh_message_t *msgout) {

    uint8_t data[MAX_COMMAND_SIZE];
    uint16_t len = 0;

    const char delim[2] = " ";
    char *tok;

    // If this isn't an ASCII message, just copy it over
    if (!fh_message_is_ascii(msgin)) {
        fh_message_copy_from(msgout, msgin);
        return 0;
    }

    // Pull out the ASCII command+args from the message
    char *cmd = (char *)fh_message_getData(msgin);    

    // Grab the first token from the command
    tok = strtok(cmd, delim);
    msg_dict_entry_t key = {upper(tok), 0, 0};
    msg_dict_entry_t *entry;
  
    // Search for the string in the message dictionary
    entry = bsearch(&key, msg_dict->e, msg_dict->len,
                    sizeof(msg_dict_entry_t), msg_dict_compare);

    if (entry) {
        // Pull out and convert the arguments
        tok = strtok(NULL, delim);
        while (tok != NULL) {
            encode_int32(data, strtol(tok, (char**)NULL, 10), len);
            tok = strtok(NULL, delim);
            len += sizeof(int32_t);
        }
        fh_message_init_full(msgout, entry->mt, entry->mst, data, len);
        return 0;
    }
    else {
        return -1;
    }

}
