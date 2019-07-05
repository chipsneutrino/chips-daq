/**
 * fh_ascii.h
 *
 * Ascii subsytem constants
 */

#ifndef FH_ASCII_H_INCLUDED
#define FH_ASCII_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

char *ascii_ok;
size_t ascii_ok_len;
void init_ascii_unspecified_err(char* buf, size_t buf_len);
void init_ascii_invalid_command_err(char* buf, size_t buf_len, char* cmd);
void init_ascii_invalid_count_err(char* buf, size_t buf_len);
void init_ascii_invalid_number_err(char* buf, size_t buf_len);
void init_ascii_out_of_range_err(char* buf, size_t buf_len);
void init_ascii_command_failed_err(char* buf, size_t buf_len);
void init_ascii_busy_err(char* buf, size_t buf_len);

void init_ascii_no_translate_err(char* buf, size_t buf_len);
void init_ascii_xlat_err(char* buf, size_t buf_len, char *file, int line);
void init_ascii_format_err(char* buf, size_t buf_len, char fmt_c);
void init_ascii_overlow_err(char* buf, size_t buf_len, size_t used, size_t available, size_t needed);
void translate_binary_err(fh_message_t *binmsg, char *buf, size_t buf_len);

size_t strnlen (const char *str, size_t n);

#ifdef __cplusplus
}
#endif

#endif
