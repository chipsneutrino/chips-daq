
/**
 * fh_ascii.c
 *
 * Common elements of the ascii subsytem.
 */

#include "../fh_classes.h"

// prompt
char *ascii_ok = "OK";
size_t ascii_ok_len = 3;

// errors (newline managed elsewhere)
char *ascii_unspecified_err = "?UNSPECIFIED_ERROR";
char *ascii_invalid_count_err = "?INVALID ARGUMENT COUNT";
char *ascii_invalid_number_err = "?INVALID NUMBER";
char *ascii_out_of_range_err = "?OUT OF RANGE";
char *ascii_command_failed_err = "?COMMAND FAILED";
char *ascii_busy_err = "?BUSY";


void init_ascii_unspecified_err(char* buf, size_t buf_len)
{
   snprintf(buf, buf_len, "%s", ascii_unspecified_err);
}

void init_ascii_invalid_command_err(char* buf, size_t buf_len, char* cmd)
{
   snprintf(buf, buf_len, "?INVALID COMMAND [%s]", cmd);
}

void init_ascii_invalid_count_err(char* buf, size_t buf_len)
{
   snprintf(buf, buf_len, "%s", ascii_invalid_count_err);
}

void init_ascii_invalid_number_err(char* buf, size_t buf_len)
{
   snprintf(buf, buf_len, "%s", ascii_invalid_number_err);
}

void init_ascii_out_of_range_err(char* buf, size_t buf_len)
{
   snprintf(buf, buf_len, "%s", ascii_out_of_range_err);
}

void init_ascii_command_failed_err(char* buf, size_t buf_len)
{
   snprintf(buf, buf_len, "%s", ascii_command_failed_err);
}

void init_ascii_busy_err(char* buf, size_t buf_len)
{
   snprintf(buf, buf_len, "%s", ascii_busy_err);
}

void init_ascii_no_translate_err(char* buf, size_t buf_len)
{
   snprintf(buf, buf_len, "?NO ASCII COMMAND TRANSLATION");
}

void init_ascii_xlat_err(char* buf, size_t buf_len, char *file, int line)
{
   snprintf(buf, buf_len, "?TRANSLATION_ERROR [%s:%d]", file, line);
}

void
init_ascii_overlow_err(char* buf, size_t buf_len, size_t used, size_t available, size_t needed)
{
   snprintf(buf, buf_len, "?XLAT OVERFLOW [%zu] OF [%zu], NEED [%zu]\n", used, available, needed);
}

void init_ascii_format_err(char* buf, size_t buf_len, char fmt_c)
{
   snprintf(buf, buf_len, "?BAD FORMAT CHAR [%c]", fmt_c);
}

void
translate_binary_err(fh_message_t *binmsg, char *buf, size_t buf_len)
{
    uint8_t mt = fh_message_getType(binmsg);
    uint8_t mst = fh_message_getSubtype(binmsg);

    if (mt == 0) {
        switch (mst) {
          case ERR_UNSPECIFIED:
          {
            init_ascii_unspecified_err(buf, buf_len);
            break;
          }
          case ERR_SUBSYSTEM_SPECIFIC:
          {
            //subsystem may generate its own error strings

            // todo check that content is of the form "?ASCII PRINTABLE\0"
            uint8_t *msg = fh_message_getData(binmsg);
            size_t len = fh_message_dataLen(binmsg);

            if(len <= buf_len)
            {
               memcpy(buf, msg, len);
            }
            else
            {
              snprintf(buf, buf_len, "?SUBSYSTEM_SPECIFIC_ERROR");
            }

            break;
          }
          case ERR_BAD_ARG_CNT:
          {
            init_ascii_invalid_count_err(buf, buf_len);
            break;
          }
          case ERR_BAD_VALUE:
          {
            init_ascii_invalid_number_err(buf, buf_len);
            break;
          }
          case ERR_OUT_OF_RANGE:
          {
            init_ascii_out_of_range_err(buf, buf_len);
            break;
          }
          case ERR_BAD_CMD:
          {
            snprintf(buf, buf_len, "?INVALID_CMD");
            break;
          }
          case ERR_CMD_FAILED:
          {
            init_ascii_command_failed_err(buf, buf_len);
            break;
          }
          case ERR_BUSY:
          {
            init_ascii_busy_err(buf, buf_len);
            break;
          }
        default: {
            // todo map specific error codes to strings
            snprintf(buf, buf_len, "?UNTRANSLATED BINARY ERROR [%d, %d]", mt, mst);
        }
        }
    }
    else {
        // was not an error message
        init_ascii_xlat_err(buf, buf_len, __FILE__, __LINE__);
    }
}

// not available in all target clibs
size_t
strnlen (const char *str, size_t n)
{
  const char *start = str;

  while (n-- > 0 && *str)
    str++;

  return str - start;
}
