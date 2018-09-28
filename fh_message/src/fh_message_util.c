/**
 * fh_message_util.h
 *
 * Utility methods for working with messages.
 */

#include "fh_classes.h"
#include <stdarg.h>

 // initialize an ASCII message
void
fh_message_init_ascii_msg(fh_message_t *self, const char *str, ...)
{
    va_list arg;

    va_start(arg, str);
    char buf[1024];
    vsnprintf(buf, 1024, str, arg);
    va_end(arg);

    fh_message_setType(self, 0);
    fh_message_setSubtype(self, 1);
    // include null termination
    fh_message_setData(self, (uint8_t *)buf, strlen(buf)+1);
}

// Is message a special ASCII type?
int
fh_message_is_ascii(fh_message_t *self) {
    return ((fh_message_getType(self) == 0) && (fh_message_getSubtype(self) == 1));
}

// encode a uint16_t in big-endian
void
encode_uint16(uint8_t buf[], uint16_t val, uint16_t pos)
{
   buf[pos++] = (uint8_t)((val >> 8) & 0xff);
   buf[pos++] = (uint8_t)(val & 0xff);
}


// decode a uint16_t encoded as big-endian from a message
uint16_t
extract_uint16(fh_message_t *msg, uint16_t pos)
{
  assert(fh_message_dataLen(msg) >= (pos + 4) );
  return extract_uint16_b(fh_message_getData(msg), pos);
}

// decode a uint16_t encoded as big-endian from a buffer
uint16_t
extract_uint16_b(uint8_t buf[], uint16_t pos)
{
  uint16_t field = 0;
  buf += pos;
  field |= (*buf++ << 8);
  field |= *buf++;
  
  return field;
}

// encode a uint32_t in big-endian
void
encode_uint32(uint8_t buf[], uint32_t val, uint16_t pos)
{
   buf[pos++] = (uint8_t)((val >> 24) & 0xff);
   buf[pos++] = (uint8_t)((val >> 16) & 0xff);
   buf[pos++] = (uint8_t)((val >> 8) & 0xff);
   buf[pos++] = (uint8_t)(val & 0xff);
}


// decode a uint32_t encoded as big-endian from a buffer
uint32_t
extract_uint32(fh_message_t *msg, uint16_t pos)
{
  assert(fh_message_dataLen(msg) >= (pos + 4) );
  uint8_t *data = fh_message_getData(msg);

  uint32_t field = 0;
  data += pos;
  field |= (*data++ << 24);
  field |= (*data++ << 16);
  field |= (*data++ << 8);
  field |= *data++;
  
  return field;
}

// encode a int32_t in big-endian
void
encode_int32(uint8_t buf[], int32_t val, uint16_t pos)
{
   buf[pos++] = (uint8_t)((val >> 24) & 0xff);
   buf[pos++] = (uint8_t)((val >> 16) & 0xff);
   buf[pos++] = (uint8_t)((val >> 8) & 0xff);
   buf[pos++] = (uint8_t)(val & 0xff);
}

// decode a int32_t encoded as big-endian from a buffer
int32_t
extract_int32(fh_message_t *msg, int16_t pos)
{
  assert(fh_message_dataLen(msg) >= (pos + 4) );
  uint8_t *data = fh_message_getData(msg);

  int32_t field = 0;
  data += pos;
  field |= (*data++ << 24);
  field |= (*data++ << 16);
  field |= (*data++ << 8);
  field |= *data++;
  
  return field;
}

