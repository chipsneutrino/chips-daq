#ifndef FRAME_H
#define FRAME_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define MAXBUFFER 1024
#define HDRFTR 5//size of wrapper including delimiters
#define MAXDATA (MAXBUFFER+HDRFTR+MAXBUFFER/256+10) //4 extra for cobs encode
//plus 10 just in case

#define FRAME_DELIMITER '\0'

typedef struct Frame{
	uint16_t checksum;
	char msg[MAXBUFFER]; // Fix me should be struct
	uint8_t dat[MAXDATA];
	uint8_t BusID;
	bool isCorrupt;
	bool isEncoded;
	size_t datLen;
	size_t msgLen;
} Frame;

uint16_t fletcher16(const uint8_t *input, size_t bytes);
void frameinit(Frame *f, char *mess, size_t len);//init values to sane things
void frameinit_encoded(Frame *f, uint8_t *dat, size_t datLen);
void encode(Frame *f);//encode msg into dat
int decode(Frame *f);//decode dat into msg
void printframe(Frame *f);

#endif
