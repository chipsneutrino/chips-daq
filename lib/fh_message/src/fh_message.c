/**
 * fh_message.c
 *
 * Encapsulates the on-the-wire message format.
 */

#include "fh_classes.h"

// todo make max size publicly configurable
#define FH_MSG_HDR_LEN 6
#define FH_MAX_TOTAL_MESSAGE 1024
#define FH_MAXDATA_VALUE (FH_MAX_TOTAL_MESSAGE - FH_MSG_HDR_LEN)
#define READ_TIMEOUT -1

// Message structure. In-memory format is same as on-wire format.
struct _fh_message_t {
    union HEAD {
        struct HD {
            uint8_t mt;
            uint8_t mst;
            uint8_t dlenHI;
            uint8_t dlenLO;
            uint8_t res[2];
        } hd;
        uint8_t h[FH_MSG_HDR_LEN];
    } head;
    uint8_t data[FH_MAXDATA_VALUE];
};

//  Create a new message
fh_message_t *
fh_message_new(void)
{
    fh_message_t *self = (fh_message_t *)calloc(1,sizeof(fh_message_t));
    assert(self);
    fh_message_init(self, 0, 0);
    return self;
}

//  Destroy the message
void
fh_message_destroy(fh_message_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        fh_message_t *self = *self_p;
        free(self);
        *self_p = NULL;
    }
}

// zero-out a message resulting in a message of type 0, sutbype 0
// with no data payload.
void
fh_message_clear(fh_message_t *self)
{
    memset(self->head.h, 0, FH_MSG_HDR_LEN);
    memset(self->data, 0, FH_MAXDATA_VALUE);
}

// initialze a message with a type/subtype with no data payload
void
fh_message_init(fh_message_t *self, uint8_t type, uint8_t subtype)
{
    fh_message_clear(self);
    fh_message_setType(self, type);
    fh_message_setSubtype(self, subtype);
}

// initialze a message with a type/subtype and payload data.
void
fh_message_init_full(fh_message_t *self, uint8_t type, uint8_t subtype, const uint8_t *data, uint16_t length)
{
    fh_message_init(self, type, subtype);
    fh_message_setData(self, data, length);
}

// initialize a message with the contents of another message
void
fh_message_copy_from(fh_message_t *self, fh_message_t *msg) {
    fh_message_init(self, fh_message_getType(msg), fh_message_getSubtype(msg));
    fh_message_setData(self, fh_message_getData(msg), fh_message_dataLen(msg));
}

// write the serialize message to a transport stream
int
fh_message_serialize(fh_message_t *self, fh_stream_t *dst)
{
    int towrite = fh_message_dataLen(self) + FH_MSG_HDR_LEN;
    uint8_t *dat = (uint8_t *)self;
    while (towrite > 0) {
        int written = fh_stream_write(dst, dat, towrite);
        if (written < 1) {
            return -1;
        }
        dat += written;
        towrite -= written;
    }
    return 0;
}

// consume a message from a transport stream, initializing a message with the deserialized content
int
fh_message_deserialize(fh_message_t *self, fh_stream_t *src)
{

    // read header
    uint16_t len = 0;
    while (len < FH_MSG_HDR_LEN) {
        // printf("*->* requesting %d\n", FH_MSG_HDR_LEN - len);
        int rd = fh_stream_read(src, &(self->head.h[len]), FH_MSG_HDR_LEN - len, READ_TIMEOUT);
        // printf("*->* got %d\n", rd);
        if (rd < 1) {
            return -1;
        }
        len += rd;
    }
    // read data
    uint16_t payload_len = fh_message_dataLen(self);
    len = 0;
    while (len < payload_len) {
        // printf("*->* requesting %d\n", (payload_len - len));
        int rd = fh_stream_read(src, &(self->data[len]), payload_len - len, READ_TIMEOUT);
        // printf("*->* got %d\n", rd);
        if (rd < 1) {
            return -1;
        }
        len += rd;
    }

    return 0;
}

// write the serialize message to buffer
int
fh_message_serialize_to_buf(fh_message_t *self, uint8_t *buf, size_t length)
{
    int towrite = fh_message_dataLen(self) + FH_MSG_HDR_LEN;

    if(length < towrite)
    {
        return -1;
    }

    memcpy(buf, (uint8_t *)self, towrite);
    return 0;
}

// deserialize a message from a buffer
int
fh_message_deserialize_from_buf(fh_message_t *self, const uint8_t *buf, size_t length)
{
    if(length < FH_MSG_HDR_LEN)
    {
        return -1;
    }
    
    // read header
    memcpy(&(self->head.h), buf, FH_MSG_HDR_LEN);
    
    //read data
    uint16_t payload_len = fh_message_dataLen(self);

    if(length < (FH_MSG_HDR_LEN + payload_len))
    {
        return -1;
    }
    memcpy(&(self->data), (buf + FH_MSG_HDR_LEN), payload_len);

    return 0;
}

// access the raw serialized message bytes
void
fh_message_raw(fh_message_t *self, fh_message_serialized *serialized)
{
    serialized->rawdata = (uint8_t *)self;
    serialized->length = fh_message_get_serialized_size(self);
}

// access the total size of a message in serialized form
uint32_t
fh_message_get_serialized_size(fh_message_t *self)
{
    return fh_message_dataLen(self) + FH_MSG_HDR_LEN;
}

// message type
uint8_t
fh_message_getType(fh_message_t *self)
{
    return self->head.hd.mt;
}

// message subtype
uint8_t
fh_message_getSubtype(fh_message_t *self)
{
    return self->head.hd.mst;
}

// access the message payload data
uint8_t *
fh_message_getData(fh_message_t *self)
{
    return self->data;
}

// message payload data length
uint16_t
fh_message_dataLen(fh_message_t *self)
{
    return (self->head.hd.dlenLO + (self->head.hd.dlenHI << 8));
}

// set the message type
void
fh_message_setType(fh_message_t *self, uint8_t t)
{
    self->head.hd.mt = t;
}

// set the message subtype
void
fh_message_setSubtype(fh_message_t *self, uint8_t st)
{
    self->head.hd.mst = st;
}

// set the message data payload (with copy)
void
fh_message_setData(fh_message_t *self, const uint8_t *d, uint16_t l)
{
    uint16_t len = l > FH_MAXDATA_VALUE ? FH_MAXDATA_VALUE : l;
    memcpy(self->data, d, len);
    self->head.hd.dlenLO = len & 0xff;
    self->head.hd.dlenHI = (len >> 8) & 0xff;
}

// set the message data payload length
void
fh_message_setDataLen(fh_message_t *self, uint16_t l)
{
    if (l > FH_MAXDATA_VALUE) {
        l = FH_MAXDATA_VALUE;
    }
    self->head.hd.dlenLO = l & 0xff;
    self->head.hd.dlenHI = (l >> 8) & 0xff;
}

// generate a hex dump of the serialized message to a stream
void
fh_message_hexdump(fh_message_t *self, const char *desc, FILE *fout)
{
    fh_util_hexdump(fout, desc, self, (fh_message_dataLen(self) + FH_MSG_HDR_LEN));
}

// get the maximum possible data length (bytes)
uint16_t
fh_message_getMaxDataLen(void) {
    return (uint16_t)FH_MAXDATA_VALUE;
}
