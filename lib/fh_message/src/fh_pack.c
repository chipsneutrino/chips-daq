/**
 * fh_pack.c
 *
 * Implements pack/unpack funtions.
 */
#include "fh_classes.h"
 

// Pack data into a buffer using a python pack-like
// format string and a variable argument list
//
// All data formatted to network endianess
//
//
// +---------------------------+
// |       Format Symbols      |
// +---------------------------+
// |     |     type    | size  |
// +-----+-------------+-------+
// |  x  |  1-byte pad |   1   |
// +-----+-------------+-------+
// |  b  |  int8_t     |   1   |
// +-----+-------------+-------+
// |  B  |  uint8_t    |   1   |
// +-----+-------------+-------+
// |  h  |  int16_t    |   2   |
// +-----+-------------+-------+
// |  H  |  uint16_t   |   2   |
// +-----+-------------+-------+
// |  i  |  int32_t    |   4   |
// +-----+-------------+-------+
// |  I  |  uint32_t   |   4   |
// +-----+-------------+-------+
// |  l  |  int64_t    |   8   |
// +-----+-------------+-------+
// |  L  |  uint64_t   |   8   |
// +-----+-------------+-------+
// |  s  |  string     | <var> |
// +-----+-------------+-------+
//
//
bool
fh_pack(uint8_t *dst, size_t dst_len, size_t offset, size_t *used, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    bool status = fh_vpack(dst, dst_len, offset, used, fmt, argp);

    va_end(argp);

    return status;
}

// variable argument list form of fh_pack
bool
fh_vpack(uint8_t *dst, size_t dst_len, size_t offset, size_t *used, const char *fmt, va_list argp)
{

    if (dst_len < offset) {
        return false;
    }

    const char *fmt_p;

#define ENSURE_AVAILABLE(idx, type_size, len)                                                                          \
    do {                                                                                                               \
        if ((idx + type_size) > len) {                                                                                 \
            success = false;                                                                                           \
            break;                                                                                                     \
        }                                                                                                              \
    } while (0)

    bool success = true;
    size_t idx = offset;
    for (fmt_p = fmt; *fmt_p != '\0'; fmt_p++) {
        switch (*fmt_p) {
        case 'x': {
            ENSURE_AVAILABLE(idx, 1, dst_len);
            dst[idx++] = 0;
            break;
        }
        case 'b':
        case 'B': {
            ENSURE_AVAILABLE(idx, 1, dst_len);
            uint8_t val = va_arg(argp, int);
            dst[idx++] = val;
            break;
        }
        case 'h':
        case 'H': {
            ENSURE_AVAILABLE(idx, 2, dst_len);
            uint16_t val = va_arg(argp, int);
            encode_uint16(dst, val, idx);
            idx += 2;
            break;
        }
        case 'i':
        case 'I': {
            ENSURE_AVAILABLE(idx, 4, dst_len);
            uint32_t val = va_arg(argp, int);
            encode_uint32(dst, val, idx);
            idx += 4;
            break;
        }
        case 'l':
        case 'L': {
            ENSURE_AVAILABLE(idx, 8, dst_len);
            uint64_t val = va_arg(argp, uint64_t);
            encode_uint64(dst, val, idx);
            idx += 8;
            break;
        }
        case 's': {
            char *val = va_arg(argp, char *);

            size_t max_len = (dst_len - idx - 1);
            size_t len = strnlen(val, max_len);

            ENSURE_AVAILABLE(idx, (len + 1), dst_len);

            strcpy((char *)&(dst[idx]), val); // null terminated
            idx += (len + 1);
            break;
        }
        default: {
            // unidentified symbol
            success = false;
            break;
        }
        }

        // break loop on bad format specifier
        if (!success) {
            break;
        }
    }

    *used = (idx - offset);
    return success;

#undef ENSURE_AVAILABLE
}

// Unpack data from a buffer using a python unpack-like
// format string and a variable argument list
//
// +---------------------------+
// |       Format Symbols      |
// +---------------------------+
// |     |     type    | size  |
// +-----+-------------+-------+
// |  x  |  1-byte pad |   1   |
// +-----+-------------+-------+
// |  b  |  int8_t     |   1   |
// +-----+-------------+-------+
// |  B  |  uint8_t    |   1   |
// +-----+-------------+-------+
// |  h  |  int16_t    |   2   |
// +-----+-------------+-------+
// |  H  |  uint16_t   |   2   |
// +-----+-------------+-------+
// |  i  |  int32_t    |   4   |
// +-----+-------------+-------+
// |  I  |  uint32_t   |   4   |
// +-----+-------------+-------+
// |  l  |  int64_t    |   8   |
// +-----+-------------+-------+
// |  L  |  uint64_t   |   8   |
// +-----+-------------+-------+
// |  s  |  string     | <var> |
// +-----+-------------+-------+
//
// 's' requires 2 parameters, a char* and a size_t max length.
bool
fh_unpack(const uint8_t *src, size_t src_len, size_t offset, size_t *used, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    bool status = fh_vunpack(src, src_len, offset, used, fmt, argp);

    va_end(argp);

    return status;
}

// variable argument list form of fh_pack
bool
fh_vunpack(const uint8_t *src, size_t src_len, size_t offset, size_t *used, const char *fmt, va_list argp)
{

    if (src_len < offset) {
        return false;
    }

    const char *fmt_p;

#define ENSURE_AVAILABLE(idx, type_size, len)                                                                          \
    do {                                                                                                               \
        if ((idx + type_size) > len) {                                                                                 \
            success = false;                                                                                           \
            break;                                                                                                     \
        }                                                                                                              \
    } while (0)

    bool success = true;
    size_t idx = offset;
    for (fmt_p = fmt; *fmt_p != '\0'; fmt_p++) {

        switch (*fmt_p) {
        case 'x': {
            ENSURE_AVAILABLE(idx, 1, src_len);
            idx++;
            break;
        }
        case 'b':
        case 'B': {
            ENSURE_AVAILABLE(idx, 1, src_len);
            uint8_t *dst = va_arg(argp, uint8_t *);
            *dst = src[idx++];
            break;
        }
        case 'h':
        case 'H': {
            ENSURE_AVAILABLE(idx, 2, src_len);
            uint16_t *dst = va_arg(argp, uint16_t *);
            *dst = extract_uint16_b(src, idx);
            idx += 2;
            break;
        }
        case 'i':
        case 'I': {
            ENSURE_AVAILABLE(idx, 4, src_len);
            uint32_t *dst = va_arg(argp, uint32_t *);
            *dst = extract_uint32_b(src, idx);
            idx += 4;
            break;
        }
        case 'l':
        case 'L': {
            ENSURE_AVAILABLE(idx, 8, src_len);
            uint64_t *dst = va_arg(argp, uint64_t *);
            *dst = extract_uint64_b(src, idx);
            idx += 8;
            break;
        }
        case 's': {

            char *str = (char *)&(src[idx]);
            size_t maxlen = (src_len - idx - 1);
            size_t len = strnlen(str, maxlen);
            size_t needed = len + 1;

            ENSURE_AVAILABLE(idx, needed, src_len);

            char *dst = va_arg(argp, char *);
            size_t dst_len = va_arg(argp, size_t);
            ENSURE_AVAILABLE(0, needed, dst_len);

            strcpy(str, dst); // null terminated
            idx += (len + 1);
            break;
        }
        default: {
            // unidentified symbol
            success = false;
            break;
        }
        }

        // break loop on bad format specifier
        if (!success) {
            break;
        }
    }

    *used = (idx - offset);

    return success;

#undef ENSURE_AVAILABLE
}

// Pack data into a buffer as a null-terminated ascii string using a python pack-like
// format string and a variable argument list.
//
// Format is a null terminated ascii string with each parameter seperated by a space
// character.
//
// see fh_pack()
//
// Does not support the pad format specifier 'x'.
bool
fh_pack_ascii(uint8_t *dst, size_t dst_len, size_t offset, size_t *used, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    bool status = fh_vpack_ascii(dst, offset, dst_len, used, fmt, argp);

    va_end(argp);

    return status;
}

// variable argument list form of fh_pack_ascii
bool
fh_vpack_ascii(uint8_t *dst, size_t dst_len, size_t offset, size_t *used, const char *fmt, va_list argp)
{

    if (dst_len < offset) {
        return false;
    }

    const char *fmt_p;

#define ENSURE_AVAILABLE(idx, type_size, len)                                                                          \
    do {                                                                                                               \
        if ((idx + type_size) > len) {                                                                                 \
            success = false;                                                                                           \
            break;                                                                                                     \
        }                                                                                                              \
    } while (0)

    bool success = true;
    uint16_t idx = offset;
    for (fmt_p = fmt; *fmt_p != '\0'; fmt_p++) {

        switch (*fmt_p) {

        case 'b': {
            int8_t val = va_arg(argp, int);
            int needed = snprintf( (char *)&(dst[idx]), (dst_len - idx), " %"PRId8"", val);
            ENSURE_AVAILABLE(idx, needed, dst_len); // post-checked, but effective
            idx += needed;
            break;
        }
        case 'B': {
            uint8_t val = va_arg(argp, int);
            int needed = snprintf( (char *)&(dst[idx]), (dst_len - idx), " %"PRIu8"", val);
            ENSURE_AVAILABLE(idx, needed, dst_len); // post-checked, but effective
            idx += needed;
            break;
        }
        case 'h': {
            int16_t val = va_arg(argp, int);
            int needed = snprintf((char *)&(dst[idx]), (dst_len - idx), " %" PRId16 "", val);
            ENSURE_AVAILABLE(idx, needed, dst_len); // post-checked, but effective
            idx += needed;
            break;
        }
        case 'H': {
            uint16_t val = va_arg(argp, int);
            int needed = snprintf((char *)&(dst[idx]), (dst_len - idx), " %" PRIu16 "", val);
            ENSURE_AVAILABLE(idx, needed, dst_len); // post-checked, but effective
            idx += needed;
            break;
        }
        case 'i': {
            int32_t val = va_arg(argp, int32_t);
            int needed = snprintf((char *)&(dst[idx]), (dst_len - idx), " %" PRId32 "", val);
            ENSURE_AVAILABLE(idx, needed, dst_len); // post-checked, but effective
            idx += needed;
            break;
        }

        case 'I': {
            uint32_t val = va_arg(argp, uint32_t);
            int needed = snprintf((char *)&(dst[idx]), (dst_len - idx), " %" PRIu32 "", val);
            ENSURE_AVAILABLE(idx, needed, dst_len); // post-checked, but effective
            idx += needed;
            break;
        }
        case 'l': {
            // NOTE: Not supported in newlib nano
            // todo
            int64_t val = va_arg(argp, int64_t);
            int needed = snprintf((char *)&(dst[idx]), (dst_len - idx), " %" PRId64 "", val);
            ENSURE_AVAILABLE(idx, needed, dst_len); // post-checked, but effective
            idx += needed;
            break;
        }
        case 'L': {
            // NOTE: Not supported in newlib nano
            // todo
            uint64_t val = va_arg(argp, uint64_t);
            int needed = snprintf((char *)&(dst[idx]), (dst_len - idx), " %" PRIu64 "", val);
            ENSURE_AVAILABLE(idx, needed, dst_len); // post-checked, but effective
            idx += needed;
            break;
        }
        case 's': {
            char *val = va_arg(argp, char *);

            size_t max_len = (dst_len - idx - 1);
            size_t len = strnlen(val, max_len);

            ENSURE_AVAILABLE(idx, (len + 1), dst_len);

            memcpy((char *)&(dst[idx]), " ", 1);
            idx++;
            memcpy((char *)&(dst[idx]), val, len);
            idx += len;
            break;
        }
        default: {
            // unidentified symbol
            success = false;
            break;
        }
        }

        // break loop on bad format specifier
        if (!success) {
            break;
        }
    }

    *used = (idx - offset);
    return success;

#undef ENSURE_AVAILABLE
}

bool
fh_pack_convert_ascii2bin(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used,
                          const char *fmt, char *err_buf, size_t err_buf_len)
{

    // require null terminated ascii payloads
    if(src_len != 0 && (src[src_len - 1] != '\0') )
    {
            init_ascii_xlat_err(err_buf, err_buf_len, __FILE__, __LINE__);
            return false;
    }

#define ENSURE_AVAILABLE(idx, type_size, maxsize, isSrc)                                                               \
    do {                                                                                                               \
        if ((idx + type_size) > maxsize) {                                                                             \
            /*printf(err_buf, err_buf_len, "OVERFLOW at [%d] of [%zu], need [%d]\n", idx, maxsize, type_size);*/       \
            if (isSrc) {                                                                                               \
                init_ascii_invalid_count_err(err_buf, err_buf_len);                                                    \
            }                                                                                                          \
            else {                                                                                                     \
                init_ascii_overlow_err(err_buf, err_buf_len, idx, maxsize, type_size);                                 \
            }                                                                                                          \
            success = false;                                                                                           \
            break;                                                                                                     \
        }                                                                                                              \
    } while (0)

    bool success = true;
    const char *fmt_p;
    uint16_t dstidx = 0;
    uint16_t srcidx = 0;
    for (fmt_p = fmt; *fmt_p != '\0'; fmt_p++) {

        // advance to start of token
        while(src[srcidx] == ' ' && srcidx < src_len)
        {
            srcidx++;
        }

        switch (*fmt_p) {
        case 'b':
        case 'B': {
            ENSURE_AVAILABLE(srcidx, 2, src_len, true); // minimum required for an ascii encoded number.
            ENSURE_AVAILABLE(dstidx, 1, dst_len, false);
            uint8_t val = atoi((char*)&(src[srcidx])); //Note: space terminated, unless last field
                                            //      then null terminated
            dst[dstidx++] = val;
            break;
        }
        case 'h':
        case 'H': {
            ENSURE_AVAILABLE(srcidx, 2, src_len, true); // minimum required for an ascii encoded number.
            ENSURE_AVAILABLE(dstidx, 2, dst_len, false);
            uint16_t val = atoi((char*)&(src[srcidx])); //Note: space terminated, unless last field
                                             //      then null terminated
            encode_uint16(dst, val, dstidx);
            dstidx += 2;
            break;
        }
        case 'i':
        case 'I': {
            ENSURE_AVAILABLE(srcidx, 2, src_len, true); // minimum required for an ascii encoded number.
            ENSURE_AVAILABLE(dstidx, 4, dst_len, false);
            uint32_t val = atol((char*)&(src[srcidx])); //Note: space terminated, unless last field
                                             //      then null terminated
            encode_uint32(dst, val, dstidx);
            dstidx += 4;
            break;
        }
        case 'l':
        case 'L': {
            ENSURE_AVAILABLE(srcidx, 2, src_len, true); // minimum required for an ascii encoded number.
            ENSURE_AVAILABLE(dstidx, 8, dst_len, false);
            uint64_t val = atoll((char*)&(src[srcidx])); //Note: space terminated, unless last field
                                              //      then null terminated
            encode_uint64(dst, val, dstidx);
            dstidx += 8;
            break;
        }
        case 's': {
            ENSURE_AVAILABLE(srcidx, 2, src_len, true); // minimum required for an ascii encoded string.
            char *val = (char*)&(src[srcidx]);    // Note: space terminated, unless last field
                                                  //       then null terminated

            size_t len = strcspn(val, " "); // works for space or null terminates str

            ENSURE_AVAILABLE(dstidx, (int)len+1, dst_len, false);

            memcpy((char *)&(dst[dstidx]), val, len);
            dstidx += len;
            dst[dstidx++] = '\0'; //in binary format strings are null terminated
            break;
        }
        default: {
            // unidentified symbol
            init_ascii_format_err(err_buf, err_buf_len, *fmt_p);
            success = false;
            break;
        }
        }

        // advance past the end of token
        // Note: for final token this leave idx = src_len
        while (src[srcidx] != ' ' && srcidx < src_len) {
            srcidx++;
        }

        //break loop on bad format specifier
        if (!success) {
            break;
        }

    }

    // was source fully consumed
    if(srcidx != src_len)
    {
        init_ascii_invalid_count_err(err_buf, err_buf_len);
        success = false;
    }

    *used = dstidx;
    return success;
    
#undef ENSURE_AVAILABLE
}

bool
fh_pack_convert_bin2ascii(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len, size_t *used,
                          const char *fmt, char *err_buf, size_t err_buf_len)
{
    const char *fmt_p;

#define ENSURE_AVAILABLE(idx, type_size, maxsize)                                                                      \
    do {                                                                                                               \
        if ((idx + type_size) > maxsize) {                                                                             \
            success = false;                                                                                           \
            break;                                                                                                     \
        }                                                                                                              \
    } while (0)

    uint16_t dstidx = 0;
    uint16_t srcidx = 0;
    bool success = true;
    for (fmt_p = fmt; *fmt_p != '\0'; fmt_p++) {

        switch (*fmt_p) {
      
        case 'b': {
            ENSURE_AVAILABLE(srcidx, 1, src_len);
            int8_t val = (int8_t)src[srcidx];
            int needed = snprintf( (char *)&(dst[dstidx]), (dst_len - dstidx), " %d", val);
            ENSURE_AVAILABLE(dstidx, needed, dst_len); //post-checked, but effective
            dstidx+=needed;
            srcidx++;
            break;
        }
        case 'B': {
            ENSURE_AVAILABLE(srcidx, 1, src_len);
            uint8_t val = (int8_t)src[srcidx];
            int needed = snprintf( (char *)&(dst[dstidx]), (dst_len - dstidx), " %d", val);
            ENSURE_AVAILABLE(dstidx, needed, dst_len); //post-checked, but effective
            dstidx+=needed;
            srcidx++;
            break;
        }
        case 'h': {
            ENSURE_AVAILABLE(srcidx, 2, src_len);
            int16_t val = extract_uint16_b(src, srcidx);
            int needed = snprintf( (char *)&(dst[dstidx]), (dst_len - dstidx), " %"PRId16"", val);
            ENSURE_AVAILABLE(dstidx, needed, dst_len); //post-checked, but effective
            dstidx+=needed;
            srcidx+=2;
            break;
        }
        case 'H': {
            ENSURE_AVAILABLE(srcidx, 2, src_len);
            uint16_t val = extract_uint16_b(src, srcidx);
            int needed = snprintf( (char *)&(dst[dstidx]), (dst_len - dstidx), " %"PRIu16"", val);
            ENSURE_AVAILABLE(dstidx, needed, dst_len); //post-checked, but effective
            dstidx+=needed;
            srcidx+=2;
            break;
        }
        case 'i': {
            ENSURE_AVAILABLE(srcidx, 4, src_len);
            int32_t val = extract_int32_b(src, srcidx);
            int needed = snprintf( (char *)&(dst[dstidx]), (dst_len - dstidx), " %"PRId32"", val);
            ENSURE_AVAILABLE(dstidx, needed, dst_len); //post-checked, but effective
            dstidx+=needed;
            srcidx+=4;
            break;
        }

        case 'I': {
            ENSURE_AVAILABLE(srcidx, 4, src_len);
            uint32_t val = extract_uint32_b(src, srcidx);
            int needed = snprintf( (char *)&(dst[dstidx]), (dst_len - dstidx), " %"PRIu32"", val);
            ENSURE_AVAILABLE(dstidx, needed, dst_len); //post-checked, but effective
            dstidx+=needed;
            srcidx+=4;
            break;
        }
        case 'l': {
            //NOTE: Not supported in newlib nano
            //todo
            ENSURE_AVAILABLE(srcidx, 8, src_len);
             int64_t val = extract_int64_b(src, srcidx);
            int needed = snprintf( (char *)&(dst[dstidx]), (dst_len - dstidx), " %"PRId64"", val);
            ENSURE_AVAILABLE(dstidx, needed, dst_len); //post-checked, but effective
            dstidx+=needed;
            srcidx+=8;
            break;
        }
        case 'L': {
            //NOTE: Not supported in newlib nano
            //todo
            ENSURE_AVAILABLE(srcidx, 8, src_len);
             uint64_t val = extract_uint64_b(src, srcidx);
            int needed = snprintf( (char *)&(dst[dstidx]), (dst_len - dstidx), " %"PRIu64"", val);
            ENSURE_AVAILABLE(dstidx, needed, dst_len); //post-checked, but effective
            dstidx+=needed;
            srcidx+=8;
            break;
        }
        case 's': {
            // todo limit using strlen(max);
            char* val = (char*)&(src[srcidx]);
            size_t len = strlen(val);

            ENSURE_AVAILABLE(dstidx, (len+1), dst_len); 

            memcpy((char *)&(dst[dstidx]), " ", 1);
            dstidx++;
            memcpy((char *)&(dst[dstidx]), val, len);
            dstidx += len;
            break;
        }
        default: {
            // unidentified symbol
            success = false;
            break;
        }
        }

        //break loop on bad format specifier
        if (!success) {
            break;
        }
    }

    // null terminate
    if (dstidx < dst_len) {
        dst[dstidx++] = '\0';
    }
    else {
        // no room!
        success = false;
    }

    // was the source consumed fully?
    if (srcidx != src_len) {
        success = false;
    }

    *used = dstidx;
    return success;
    
#undef ENSURE_AVAILABLE
}

