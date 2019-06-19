/**
 * Special class for constructing the content of a MCF message.
 */

#include "msg_writer.h"

MsgWriter::MsgWriter() 
{
	data_.clear();
}
	
void MsgWriter::writeI8(int b) {
	unsigned char c = 0xFF & b;
	data_.push_back(c);
}
	
void MsgWriter::writeU8(short b) 
{
	unsigned char c = 0xFF & b;
	data_.push_back(c);
}

void MsgWriter::writeI16(short b) 
{
	unsigned char c1 = 0xFF & (b >> 8);
	unsigned char c2 = 0xFF & b;
	data_.push_back(c1);
	data_.push_back(c2);
}
	
void MsgWriter::writeU16(int b)
{
	unsigned char c1 = 0xFF & (b >> 8);
	unsigned char c2 = 0xFF & b;
	data_.push_back(c1);
	data_.push_back(c2);
}

	
void MsgWriter::writeI32(int b)
{
	unsigned char c1 = 0xFF & (b >> 24);
	unsigned char c2 = 0xFF & (b >> 16);
	unsigned char c3 = 0xFF & (b >>  8);
	unsigned char c4 = 0xFF & b;
	data_.push_back(c1);
	data_.push_back(c2);
	data_.push_back(c3);
	data_.push_back(c4);
}
	
void MsgWriter::writeU32(long b)
{
	unsigned char c1 = 0xFF & (b >> 24);
	unsigned char c2 = 0xFF & (b >> 16);
	unsigned char c3 = 0xFF & (b >>  8);   
	unsigned char c4 = 0xFF & b;
	data_.push_back(c1);
	data_.push_back(c2);
	data_.push_back(c3);
	data_.push_back(c4);
}

	
void MsgWriter::writeI64(long b)
{
	unsigned char c1 = 0xFF & (b >> 56);
	unsigned char c2 = 0xFF & (b >> 48);
	unsigned char c3 = 0xFF & (b >> 40); 
	unsigned char c4 = 0xFF & (b >> 32);
	unsigned char c5 = 0xFF & (b >> 24);
	unsigned char c6 = 0xFF & (b >> 16);
	unsigned char c7 = 0xFF & (b >>  8);
	unsigned char c8 = 0xFF & b;
	data_.push_back(c1);
	data_.push_back(c2);
	data_.push_back(c3);
	data_.push_back(c4);
	data_.push_back(c5);
	data_.push_back(c6);
	data_.push_back(c7);
	data_.push_back(c8);
}

void MsgWriter::writeU64(long long int b)
{
	unsigned char c1 = 0xFF & (b >> 56);
	unsigned char c2 = 0xFF & (b >> 48);
	unsigned char c3 = 0xFF & (b >> 40);
	unsigned char c4 = 0xFF & (b >> 32);  
	unsigned char c5 = 0xFF & (b >> 24);
	unsigned char c6 = 0xFF & (b >> 16);
	unsigned char c7 = 0xFF & (b >>  8);    
	unsigned char c8 = 0xFF & b;
	data_.push_back(c1);
	data_.push_back(c2);
	data_.push_back(c3);
	data_.push_back(c4);
	data_.push_back(c5);
	data_.push_back(c6);
	data_.push_back(c7);
	data_.push_back(c8);  
}

void MsgWriter::writeF32(float value) 
{
	int *ivalue = (int*)(&value);  // conver bytes to int
	writeI32(*ivalue);  
}

void MsgWriter::writeF64(double value) 
{
	long int *ivalue = (long int*)(&value);  // conver bytes to long int
	writeI64(*ivalue);      
}
	
	
void MsgWriter::writeBoolean(bool b)
{
	unsigned char c = 0xFF & b;
	data_.push_back(c);
}
	
void MsgWriter::writeString(std::string mstring) 
{
	//writeUTF: Writes a string to the underlying output stream using modified UTF-8 encoding in a machine-independent manner.
	//First, two bytes are written to the output stream as if by the writeShort method giving the number of bytes to follow.
	// This value is the number of bytes actually written out, not the length of the string.
	// Following the length, each character of the string is output, in sequence,
	//  using the modified UTF-8 encoding for the character. If no exception is thrown, the counter written is incremented by
	//  the total number of bytes written to the output stream. This will be at least two plus the length of str,
	//  and at most two plus thrice the length of str.

	//  Do Ireally need it? simplified version with 1 byte / character  for the time being !!!!!!!!!!!!!!!

	short int ml = mstring.size();

	writeI16(ml);
	for(short int is=0; is<ml; ++is){
			data_.push_back((unsigned char) mstring[is]);    
	}
}
	
void MsgWriter::writeFixedArrayI8(std::vector<unsigned char> arr)
{
	std::vector<unsigned char>::iterator ai;
	for(ai = arr.begin(); ai!=arr.end(); ai++) writeI8(*ai);  
}
	

void MsgWriter::writeFixedArrayU8(std::vector<short> arr)
{
	std::vector<short>::iterator ai;
	for(ai = arr.begin(); ai!=arr.end(); ai++) writeU8(*ai);  
}
	

void MsgWriter::writeFixedArrayI16(std::vector<short> arr)
{
	std::vector<short>::iterator ai;
	for(ai = arr.begin(); ai!=arr.end(); ai++) writeI16(*ai);    
}

void MsgWriter::writeFixedArrayU16(std::vector<int> arr)
{
	std::vector<int>::iterator ai;
	for(ai = arr.begin(); ai!=arr.end(); ai++) writeU16(*ai);    
}

void MsgWriter::writeFixedArrayI32(std::vector<int> arr)
{
	std::vector<int>::iterator ai;
	for(ai = arr.begin(); ai!=arr.end(); ai++) writeI32(*ai);    
}
	
void MsgWriter::writeFixedArrayU32(std::vector<long> arr)
{
	std::vector<long>::iterator ai;
	for(ai = arr.begin(); ai!=arr.end(); ai++) writeU32(*ai);    
}

	
void MsgWriter::writeFixedArrayI64(std::vector<long> arr)
{
	std::vector<long>::iterator ai;
	for(ai = arr.begin(); ai!=arr.end(); ai++) writeI64(*ai);    
}
	
void MsgWriter::writeVarArrayI8(std::vector<unsigned char> arr)
{
	writeI16((short)arr.size());
	writeFixedArrayI8(arr);
}

void MsgWriter::writeVarArrayU8(std::vector<short> arr)
{
	writeI16((short)arr.size());
	writeFixedArrayU8(arr);
}

void MsgWriter::writeVarArrayI16(std::vector<short> arr)
{
	writeI16((short)arr.size());
	writeFixedArrayI16(arr);
}

void MsgWriter::writeVarArrayU16(std::vector<int> arr)
{
	writeI16((short)arr.size());
	writeFixedArrayU16(arr);
}

void MsgWriter::writeVarArrayI32(std::vector<int> arr)
{
	writeI16((short)arr.size());
	writeFixedArrayI32(arr);
}

void MsgWriter::writeVarArrayU32(std::vector<long> arr)
{
	writeI16((short)arr.size());
	writeFixedArrayU32(arr);
}

void MsgWriter::writeVarArrayI64(std::vector<long> arr)
{
	writeI16((short)arr.size());
	writeFixedArrayI64(arr);
}
	
/* 
void MsgWriter::writeDecimalAsLong(double value, int decimals) 
{
	writeI64 ( (long) ( value * DECIMALS[decimals] ) );
}

void MsgWriter::writeDecimalAsInt(double value, int decimals) 
{
	writeI32 ( (int) ( value * DECIMALS[decimals] ) );
}
	
void MsgWriter::writeDecimalAsShort(double value, int decimals) 
{
	writeI16 ( (short) ( value * DECIMALS[decimals] ) );
}
*/

void MsgWriter::writeIntArray(std::vector<int> arr, bool varLength) 
{
if (varLength) writeU16(arr.size());
for (int i = 0; i < arr.size(); ++i)
	writeI32(arr[i]);
}

void MsgWriter::writeI32Arr(std::vector<int> varIds) {
	writeU16(varIds.size());
	for (int i = 0; i < varIds.size(); ++i) {
		writeI32(varIds[i]);
	}
}

std::vector<unsigned char> MsgWriter::toBytes() 
{
	return data_;
}
