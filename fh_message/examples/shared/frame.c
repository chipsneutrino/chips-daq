#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "frame.h"
#include "cobs.h"

void encode(Frame *f) {
	f->dat[0] = FRAME_DELIMITER;
	f->datLen = 1;

	uint8_t dummy[MAXDATA];
	uint32_t dummylen = 0;

	dummy[0] = f->BusID;
	dummylen += 1;

	// FIX ME: use message struct with length
	// include string delimiter in data
	memcpy((void *)&(dummy[1]), (void *)(f->msg), f->msgLen);
	dummylen += f->msgLen;
	//strcpy((char *)&(dummy[1]), f->msg);
	//dummylen += strlen(f->msg)+1;

	f->checksum = fletcher16(dummy, dummylen);
	memcpy(&(dummy[dummylen]), &(f->checksum), 2);
	dummylen += 2;

	size_t cobsLen = cobs_encode(dummy, dummylen, &(f->dat[f->datLen]));
	f->datLen += cobsLen;

	f->dat[f->datLen] = FRAME_DELIMITER;
	f->datLen++;

	f->isEncoded = 1;
}


int decode(Frame *f) {
	if (f->isEncoded) {//don't decode what doesn't need decoding
		uint16_t cs = 0;
		uint8_t dummy[MAXDATA] = {0};

        // COBS-decode everything between the start/stop delimiters
		size_t cobsLen = cobs_decode(&(f->dat[1]), f->datLen-2, dummy);
		if (cobsLen == 0) {
			f->isCorrupt = 1;
			return -1;
		}
		f->msgLen = cobsLen-3; //(subtract 2 for checksum, 1 for bus id)
//printf("cobsLen is %d, whereas datLen-2 is %d\n",cobsLen, f->datLen-2);//-2 because delimiters

        // Copy over the checksum bytes
		memcpy(&(f->checksum), &(dummy[cobsLen-2]), 2);
//printf("copied checksum is %04x or rather %d\n",f->checksum, f->checksum);
		// Recalculate the checksum to verify integrity
		cs = fletcher16(&(dummy[0]), (cobsLen-2));
//printf("calculated checksum is %04x or rather %d\n",cs, cs);
//printf("The difference in copied and calculated is %04x\n",cs - f->checksum);
//printf("Here's a checksum of cobsLen-3 %d, -4 %d, -1 %d, -0 %d\n",fletcher16(dummy,cobsLen-3),fletcher16(dummy,cobsLen-4),fletcher16(dummy,cobsLen-1),fletcher16(dummy,cobsLen));
		if (cs != f->checksum) {
			f->isCorrupt = 1;
			//try and write message anyway
			f->BusID = dummy[0];
			memcpy(&(f->msg), &(dummy[1]), cobsLen-3);
			f->isEncoded = 0;
			return -2;
		}

		f->BusID = dummy[0];
		memcpy(&(f->msg), &(dummy[1]), cobsLen-3);
		f->isEncoded = 0;
	}
    return 0;
}

void printframe(Frame *f) {
	int i = 0;
	printf("isEncoded: %i\n", f->isEncoded);
	printf("isCorrupt: %i\n", f->isCorrupt);
	printf("message: %s\n", f->msg);
	printf("id: %i\n",f->BusID);
	printf("cs: %04x\n", f->checksum);
	if (!f->isCorrupt) {
		for (i=0; i < f->datLen; i++)
			printf("%x %c\n", f->dat[i], f->dat[i]);
	}
}

//efficient 8 bit implementation as found on wikipedia
uint16_t fletcher16(uint8_t const *input, size_t bytes){
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
	//second reduction step to reduce sums to 8 bits
	sum1 = (sum1 & 0xff) + (sum1 >> 8);
	sum2 = (sum2 & 0xff) + (sum2 >> 8);
//	return 1<<7;
	return (sum2 << 8) | sum1;
}

/*uint16_t fletcher16(uint8_t const *input, size_t bytes){
	uint16_t sum1 = 0;
	uint16_t sum2 = 0;
	int index;
	for(index = 0; index < bytes; ++index){
		sum1 = (sum1+input[index])%255;
		sum2 = (sum2+sum1)%255;
	}
	return (sum2<<8) | sum1;
}*/


void frameinit(Frame *f, char* mess, size_t len){
	f->BusID = 0;
	f->checksum = 0;
	f->isCorrupt = 0;
	f->isEncoded = 0;
	//eventually will have to be memcpy when
	//using struct for mess/msg
//	strcpy(f->msg, mess);
	memset(f->msg,0,1024);
	memcpy(f->msg,mess,len);
	f->msgLen = len;
	f->datLen = 0;
}

void frameinit_encoded(Frame *f, uint8_t *dat, size_t datLen) {
	frameinit(f, "", 0);
	memcpy(f->dat, dat, datLen);
	f->datLen = datLen;
	f->isEncoded = 1;
	f->msgLen = 0;
}
