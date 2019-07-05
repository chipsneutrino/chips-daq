/**
 * fh_util.h
 *
 * Utility functions.
 *
 */

#include "fh_classes.h"

// generate a fletcher-16 checksum 
// NOTE: efficient 8 bit implementation as found on wikipedia
uint16_t
fh_util_fletcher16(uint8_t const *input, size_t bytes)
{
    uint16_t sum1 = 0xff;
    uint16_t sum2 = 0xff;
    size_t tlen;

    while (bytes) {
        tlen = ((bytes >= 20) ? 20 : bytes);
        bytes -= tlen;
        do {
            sum2 += sum1 += *input++;
            tlen--;
        } while (tlen);
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }
    // second reduction step to reduce sums to 8 bits
    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);
    return (sum2 << 8) | sum1;
}

// generate a hex dump of arbitrary memory to a stream
void
fh_util_hexdump(FILE *fout, const char *desc, const void *addr, int len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *)addr;

    // Output description if given.
    if (desc != NULL) fprintf(fout, "%s:\n", desc);

    if (len == 0) {
        fprintf(fout, "  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        fprintf(fout, "  NEGATIVE LENGTH: %i\n", len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0) fprintf(fout, "  %s\n", buff);

            // Output the offset.
            fprintf(fout, "  %04x ", i);
        }

        // Now the hex code for the specific character.
        fprintf(fout, " %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        fprintf(fout, "   ");
        i++;
    }

    // And print the final ASCII bit.
    fprintf(fout, "  %s\n", buff);
}


// generate a hex dump of arbitrary memory to a C string in a buffer
int
fh_util_hexdump_b(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len)
{

    if (src_len == 0) {
        return 0;
    }

    // each source byte expands to '0xhh' + [' '| '\n' | '\0']
    size_t required = src_len * 5;
    if (dst_len < required) {
        return required - dst_len; // amount of underflow encoded in return value
    }

    int dst_idx = 0;
    // Process every byte in the data.
    for (int i = 0; i < src_len; i++) {

        int used = snprintf((char *)&dst[dst_idx], 3, "%02x", src[i]);

        // should never happen
        if (used != 2) { return -1; }
        
        dst_idx += 2;

        // replace snprintf null termination with '\n' or ' '
        if ((i % 15) == 0 && i > 0) {
            dst[dst_idx++] = '\n';
        }
        else {
            dst[dst_idx++] = ' ';
        }
    }

    // replace final space or newline with null terminiation
    dst[dst_idx-1] = '\0';

    return dst_idx;
}

// self test
void
fh_util_test (bool verbose)
{

}
