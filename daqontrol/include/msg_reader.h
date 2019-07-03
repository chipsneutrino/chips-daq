/**
 * Special class for reading the content of a message.
 */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

class MsgReader {
public:
	MsgReader() {};

	MsgReader(std::vector<unsigned char> data);

	void fromBuffer(std::vector<unsigned char> data);

	/// Number of bytes from current position to data end
	int available();

	/// Start iterating from the beginning of the data again
	void rewind();

	/// Skip n bytes of the data
	void skip(int bytes);

	/// ReadByte: Reads and returns one input byte. The byte is treated as a signed value in the range -128 through 127
	unsigned char readI8();

	/// ReadUnsignedByte: Reads one input byte, zero-extends it to type int, and returns the result, which is therefore in the range 0 through 255.
	short int readU8();

	/// ReadShort: Reads two input bytes and returns a short value. If a is first byte, the value returned is (short)((a << 8) | (b & 0xff)) 
	short int readI16();

	/// ReadUnsignedShort: Reads two input bytes and returns an int value in the range 0 through 65535.
	// Let a be the first byte read and b be the second byte. The value returned is:  (((a & 0xff) << 8) | (b & 0xff))
	int readU16();
	int readI32(); 
	long int readU32();
	long int readI64();
	long long  int  readU64();

	std::string readString();

	bool readBool();

	float readF32();
	double readF64();

private:
	std::vector<unsigned char>          	data_;
	std::vector<unsigned char>::iterator 	it_;
	static const unsigned long long int  	U64_MASK = 0xFFFFFFFFFFFFFFFF;  // ?????????
	int length;
};