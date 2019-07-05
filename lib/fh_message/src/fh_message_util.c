/**
 * fh_message_util.c
 *
 * Utility methods for working with messages.
 */

#include "fh_classes.h"
#include <stdarg.h>

// encode a string into the message data
void
fh_encode_ascii(fh_message_t *self, const char *str, ...)
{

  uint8_t *dst = fh_message_getData(self);
  size_t max_len = fh_message_getMaxDataLen();

    va_list arg;

    va_start(arg, str);
    int used = vsnprintf((char *)dst, max_len, str, arg);
    va_end(arg);

    if (used > 0 && used < max_len) {
        fh_message_setDataLen(self, used + 1);
    }
    else {
        // vsnprintf failed or the msg size was too small
        fh_message_setDataLen(self, 0);
    }
}

// encode an int8_t
void
encode_int8(uint8_t buf[], int8_t val, uint16_t pos)
{
   buf[pos] = val;
}

// decode an int8_t from a messgae
int8_t
extract_int8(fh_message_t *msg, uint16_t pos)
{
  assert(fh_message_dataLen(msg) >= (pos + 1) );
  uint8_t *data = fh_message_getData(msg);
  return (int8_t)data[pos];
}

// encode a uint8_t
void
encode_uint8(uint8_t buf[], uint8_t val, uint16_t pos)
{
   buf[pos] = val;
}

// decode a uint8_t from a messgae
uint8_t
extract_uint8(fh_message_t *msg, uint16_t pos)
{
  assert(fh_message_dataLen(msg) >= (pos + 1) );
  uint8_t *data = fh_message_getData(msg);
  return data[pos];
}

// encode a uint16_t in big-endian
void
encode_uint16(uint8_t buf[], uint16_t val, uint16_t pos)
{
   buf[pos++] = (uint8_t)((val >> 8) & 0xff);
   buf[pos++] = (uint8_t)(val & 0xff);
}

// encode a uint16_t in little-endian
void
encode_uint16_le(uint8_t buf[], uint16_t val, uint16_t pos)
{
   buf[pos++] = (uint8_t)(val & 0xff);
   buf[pos++] = (uint8_t)((val >> 8) & 0xff);
}


// decode a uint16_t encoded as big-endian from a message
uint16_t
extract_uint16(fh_message_t *msg, uint16_t pos)
{
  assert(fh_message_dataLen(msg) >= (pos + 2) );
  return extract_uint16_b(fh_message_getData(msg), pos);
}

// decode a uint16_t encoded as little-endian from a message
uint16_t
extract_uint16_le(fh_message_t *msg, uint16_t pos)
{
  assert(fh_message_dataLen(msg) >= (pos + 2) );
  return extract_uint16_ble(fh_message_getData(msg), pos);
}

// decode a uint16_t encoded as big-endian from a buffer
uint16_t
extract_uint16_b(const uint8_t buf[], uint16_t pos)
{
  uint16_t field = 0;
  buf += pos;
  field |= (*buf++ << 8);
  field |= *buf++;
  
  return field;
}

// decode a uint16_t encoded as little-endian from a buffer
uint16_t
extract_uint16_ble(const uint8_t buf[], uint16_t pos)
{
  uint16_t field = 0;
  buf += pos;
  field |= *buf++;
  field |= (*buf++ << 8);
  
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

// decode a uint32_t encoded as big-endian from a buffer
uint32_t
extract_uint32_b(const uint8_t buf[], uint16_t pos)
{
  uint32_t field = 0;
  buf += pos;
  field |= (*buf++ << 24);
  field |= (*buf++ << 16);
  field |= (*buf++ << 8);
  field |= *buf++;
  
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

// decode an int32_t encoded as big-endian from a buffer
int32_t
extract_int32_b(const uint8_t buf[], uint16_t pos)
{
  int32_t field = 0;
  buf += pos;
  field |= (*buf++ << 24);
  field |= (*buf++ << 16);
  field |= (*buf++ << 8);
  field |= *buf++;
  
  return field;
}

// encode a uint64_t in big-endian
void
encode_uint64(uint8_t buf[], uint64_t val, uint16_t pos)
{
   buf[pos++] = (uint8_t)((val >> 56) & 0xff);
   buf[pos++] = (uint8_t)((val >> 48) & 0xff);
   buf[pos++] = (uint8_t)((val >> 40) & 0xff);
   buf[pos++] = (uint8_t)((val >> 32) & 0xff);
   buf[pos++] = (uint8_t)((val >> 24) & 0xff);
   buf[pos++] = (uint8_t)((val >> 16) & 0xff);
   buf[pos++] = (uint8_t)((val >> 8) & 0xff);
   buf[pos++] = (uint8_t)(val & 0xff);
}

// decode a uint64_t encoded as big-endian from a message
uint64_t
extract_uint64(fh_message_t *msg, uint16_t pos)
{
  assert(fh_message_dataLen(msg) >= (pos + 8) );
  uint8_t *data = fh_message_getData(msg);

  uint64_t field = 0;
  data += pos;
  field |= ((uint64_t)*data++ << 56);
  field |= ((uint64_t)*data++ << 48);
  field |= ((uint64_t)*data++ << 40);
  field |= ((uint64_t)*data++ << 32);
  field |= (*data++ << 24);
  field |= (*data++ << 16);
  field |= (*data++ << 8);
  field |=  *data++;
  
  return field;
}

// decode a uint64_t encoded as big-endian from a buffer
uint64_t
extract_uint64_b(const uint8_t buf[], uint16_t pos)
{
  uint64_t field = 0;
  buf += pos;
  field |= ((uint64_t)*buf++ << 56);
  field |= ((uint64_t)*buf++ << 48);
  field |= ((uint64_t)*buf++ << 40);
  field |= ((uint64_t)*buf++ << 32);
  field |= (*buf++ << 24);
  field |= (*buf++ << 16);
  field |= (*buf++ << 8);
  field |=  *buf++;
  
  return field;
}

// encode a int64_t in big-endian
void
encode_int64(uint8_t buf[], int64_t val, uint16_t pos)
{
   buf[pos++] = (uint8_t)((val >> 56) & 0xff);
   buf[pos++] = (uint8_t)((val >> 48) & 0xff);
   buf[pos++] = (uint8_t)((val >> 40) & 0xff);
   buf[pos++] = (uint8_t)((val >> 32) & 0xff);
   buf[pos++] = (uint8_t)((val >> 24) & 0xff);
   buf[pos++] = (uint8_t)((val >> 16) & 0xff);
   buf[pos++] = (uint8_t)((val >> 8) & 0xff);
   buf[pos++] = (uint8_t)(val & 0xff);
}

// decode a int64_t encoded as big-endian from a message
int64_t
extract_int64(fh_message_t *msg, int16_t pos)
{
  assert(fh_message_dataLen(msg) >= (pos + 8) );
  uint8_t *data = fh_message_getData(msg);

  int64_t field = 0;
  data += pos;
  field |= ((uint64_t)*data++ << 56);
  field |= ((uint64_t)*data++ << 48);
  field |= ((uint64_t)*data++ << 40);
  field |= ((uint64_t)*data++ << 32);
  field |= (*data++ << 24);
  field |= (*data++ << 16);
  field |= (*data++ << 8);
  field |= *data++;
  
  return field;
}

// decode an int64_t encoded as big-endian from a buffer
int64_t
extract_int64_b(const uint8_t buf[], uint16_t pos)
{
  int64_t field = 0;
  buf += pos;
  field |= ((uint64_t)*buf++ << 56);
  field |= ((uint64_t)*buf++ << 48);
  field |= ((uint64_t)*buf++ << 40);
  field |= ((uint64_t)*buf++ << 32);
  field |= (*buf++ << 24);
  field |= (*buf++ << 16);
  field |= (*buf++ << 8);
  field |=  *buf++;
  
  return field;
}

// encode data into message payload using packe semantics
bool
fh_msg_pack(fh_message_t *msg, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    size_t used;
    bool status = fh_vpack(fh_message_getData(msg), fh_message_getMaxDataLen(), 0, &used, fmt, argp);
    if(status)
    {
       fh_message_setDataLen(msg, used);
    }
    else
    {
      fh_message_setDataLen(msg, 0);
    }


    va_end(argp);

    return status;
}

// initialize a message using pack semantics
bool
fh_msg_pack_full(fh_message_t *msg, uint8_t mt, uint8_t mst, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    fh_message_init(msg, mt, mst);

    size_t used;
    bool status = fh_vpack(fh_message_getData(msg), fh_message_getMaxDataLen(), 0, &used, fmt, argp);
    if(status)
    {
       fh_message_setDataLen(msg, used);
    }
    else
    {
      fh_message_setDataLen(msg, 0);
    }


    va_end(argp);

    return status;
}

// decode data from a message payload using unpack semantics
bool
fh_msg_unpack(fh_message_t *msg, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    const uint8_t *src = fh_message_getData(msg);
     size_t src_len = fh_message_dataLen(msg);
   size_t used;

    bool status = fh_vunpack(src, src_len, 0, &used, fmt, argp);

    va_end(argp);

    return status;
}
