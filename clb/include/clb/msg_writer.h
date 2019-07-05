/**
 * Special class for constructing the content of a MCF message.
 * 
 */

#pragma once

#include <vector>
#include <string>

class MsgWriter  
{
public:
	MsgWriter();

	void writeI8(int b);
	void writeU8(short b);
	void writeI16(short b); 
	void writeU16(int b);
	void writeI32(int b);
	void writeU32(long b);
	void writeI64(long b);	
	void writeU64(long long int bi);
	void writeF32(float value);
	void writeF64(double value);

	void writeBoolean(bool b);

	void writeString(std::string mstring);

	void writeFixedArrayI8(std::vector<unsigned char> arr);
	void writeFixedArrayU8(std::vector<short> arr);
	void writeFixedArrayI16(std::vector<short> arr);
	void writeFixedArrayU16(std::vector<int> arr);
	void writeFixedArrayI32(std::vector<int> arr);
	void writeFixedArrayU32(std::vector<long> arr);
	void writeFixedArrayI64(std::vector<long> arr);

	void writeVarArrayI8(std::vector<unsigned char> arr);
	void writeVarArrayU8(std::vector<short> arr);
	void writeVarArrayI16(std::vector<short> arr);
	void writeVarArrayU16(std::vector<int> arr);
	void writeVarArrayI32(std::vector<int> arr);
	void writeVarArrayU32(std::vector<long> arr);
	void writeVarArrayI64(std::vector<long> arr);

	/*
	void writeDecimalAsLong(double value, int decimals);
	void writeDecimalAsInt(double value, int decimals);
	void writeDecimalAsShort(double value, int decimals);
	*/

	void writeIntArray(std::vector<int> arr, bool varLength);
	void writeI32Arr(std::vector<int> varIds);

	std::vector<unsigned char> toBytes();

private:

	std::vector<unsigned char> data_;
	std::vector<unsigned char>::iterator _it;

	/* 
	static const long DECIMALS[19] = {
		0L,
		10L,
		100L,
		1000L,
		10000L,
		100000L,
		1000000L,
		10000000L,
		100000000L,
		1000000000L,
		10000000000L,
		100000000000L,
		1000000000000L,
		10000000000000L,
		100000000000000L,
		1000000000000000L,
		10000000000000000L,
		100000000000000000L,
		1000000000000000000L
	}; 	
	*/
};
