/**
 * Special class for reading the content of a message.
 */

#include "msg_reader.h"
  
MsgReader::MsgReader(std::vector<unsigned char> data) 
{
	data_ = data;
	it_ = data_.begin();
}
  
int MsgReader::available() 
{
  	return std::distance(it_, data_.end());
}
  
void MsgReader::rewind() 
{
	it_ = data_.begin();
}

void MsgReader::skip(int bytes) 
{
  	it_ = it_ + bytes;
}
  
unsigned char MsgReader::readI8()
{
	char value = (char)*it_;
	it_++;
	return value;
}
  
short int MsgReader::readU8()
{
	unsigned char value = *it_;  
	it_++;
	return value;
}

short int MsgReader::readI16()
{
	unsigned char a = *it_;
	it_++;
	unsigned char b = *it_;
	it_++;
	short int value = 0xFFFF & ( (a << 8) | b );
	return value;
}
  
int MsgReader::readU16()
{
	unsigned char a = *it_;   
	it_++;
	unsigned char b = *it_; 
	it_++;
	short int value = 0xFFFF & ( (a << 8) | b );
	return value;
}

int MsgReader::readI32()
{
	//readInt: Reads four input bytes and returns an int value. Let a-d be the first through fourth bytes read.
	// The value returned is : (((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) <<  8) | (d & 0xff))

	unsigned char a = *it_;
	it_++;
	unsigned char b = *it_; 
	it_++;
	unsigned char c = *it_;   
	it_++;
	unsigned char d = *it_; 
	it_++;
	
	int value = 0xFFFFFFFF & ( ((a & 0xFF) << 24) | ((b & 0xFF) << 16) | ((c & 0xFF) <<  8) | (d & 0xff) );
	//int value =  ( ((a & 0xFF) << 24)  |  ((b & 0xFF) << 16)  | ((c & 0xFF) <<  8) | (d & 0xff) );  // No Mask for signed values????
	return value;
}

long int MsgReader::readU32()
{
	//readInt: Reads four input bytes and returns an int value. Let a-d be the first through fourth bytes read.
	// The value returned is : (((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) <<  8) | (d & 0xff))

	unsigned char a = *it_;
	it_++;
	unsigned char b = *it_; 
	it_++;
	unsigned char c = *it_;   
	it_++;
	unsigned char d = *it_; 
	it_++;

	long int value = 0xFFFFFFFFL & ( ((a & 0xFF) << 24)  |  ((b & 0xFF) << 16)  | ((c & 0xFF) <<  8) | (d & 0xff) );
	return value;
}
  
long int MsgReader::readI64() 
{
	//readLong:  Reads eight input bytes and returns a long value. Let a-h be the first through eighth bytes read.
	// The value returned is:
	// (((long)(a & 0xff) << 56) |
	//  ((long)(b & 0xff) << 48) |
	//  ((long)(c & 0xff) << 40) |
	//  ((long)(d & 0xff) << 32) |
	//  ((long)(e & 0xff) << 24) |
	//  ((long)(f & 0xff) << 16) |
	//  ((long)(g & 0xff) <<  8) |
	//  ((long)(h & 0xff)))

	unsigned char a = *it_;
	it_++;
	unsigned char b = *it_; 
	it_++;
	unsigned char c = *it_;   
	it_++;
	unsigned char d = *it_; 
	it_++;
	unsigned char e = *it_;
	it_++;
	unsigned char f = *it_; 
	it_++;
	unsigned char g = *it_;   
	it_++;
	unsigned char h = *it_; 
	it_++;

	//  long int value =    // No mask for signed values????? 
	long int value =  0xFFFFFFFFFFFFFFFF &
	(((long)(a & 0xFF) << 56) |
		((long)(b & 0xFF) << 48) |
		((long)(c & 0xFF) << 40) |
		((long)(d & 0xFF) << 32) |
		((long)(e & 0xFF) << 24) |
		((long)(f & 0xFF) << 16) |
		((long)(g & 0xFF) <<  8) |
		((long)(h & 0xFF)));

	return value;
}

long long int MsgReader::readU64()
{
	//readLong:  Reads eight input bytes and returns a long value. Let a-h be the first through eighth bytes read.
	// The value returned is:
	// (((long)(a & 0xff) << 56) |
	//  ((long)(b & 0xff) << 48) |
	//  ((long)(c & 0xff) << 40) |
	//  ((long)(d & 0xff) << 32) |
	//  ((long)(e & 0xff) << 24) |
	//  ((long)(f & 0xff) << 16) |
	//  ((long)(g & 0xff) <<  8) |
	//  ((long)(h & 0xff)))

	unsigned char a = *it_;
	it_++;
	unsigned char b = *it_; 
	it_++;
	unsigned char c = *it_;   
	it_++;
	unsigned char d = *it_; 
	it_++;
	unsigned char e = *it_;
	it_++;
	unsigned char f = *it_; 
	it_++;
	unsigned char g = *it_;   
	it_++;
	unsigned char h = *it_; 
	it_++;

	long long int value =  U64_MASK  & 
	(((long)(a & 0xFF) << 56) |
		((long)(b & 0xFF) << 48) |
		((long)(c & 0xFF) << 40) |
		((long)(d & 0xFF) << 32) |
		((long)(e & 0xFF) << 24) |
		((long)(f & 0xFF) << 16) |
		((long)(g & 0xFF) <<  8) |
		((long)(h & 0xFF)));

	return value;
}

std::string MsgReader::readString() 
{
	//?????????
	////  Do Ireally need it? simplified version with 1 byte / character  for the time being !!!!!!!!!!!!!!!

	short int msize =  readI16();

	std::string mstring = "";
	
	if(msize>0) {
		for(int is=0; is<msize; ++is){      
		mstring+= *it_;
		it_++;
		}
		return mstring;
	} else {
		throw std::runtime_error("Wrong String Message Format");
	}
}

bool MsgReader::readBool() 
{
	return readI8() != 0;
}

float MsgReader::readF32() 
{
	//readFloat:  Reads four input bytes and returns a float value.
	// It does this by first constructing an int value in exactly the manner of the readInt method,
	//  then converting this int value to a float in exactly the manner of the method Float.intBitsToFloat.
	
		int ivalue = readI32();
	//Convert int bytes to float equivalent; Foun at https://stackoverflow.com/questions/30610222/c-equivalent-of-javas-float-intbitstofloat
	//  Use union instead????
	float *fvalue = (float*)(&ivalue);  
	return *fvalue;
}

double MsgReader::readF64() 
{
	/*	  try {
		return _dis.readDouble();
		} catch (IOException e) {
		throw new MessageException("Decoding failed", e);
		}*/

		//readDouble:  Reads eight input bytes and returns a double value.
		//  It does this by first constructing a long value in exactly the manner of the readlong method,
		// then converting this long value to a double in exactly the manner of the method Double.longBitsToDouble.

	long int lvalue = readI64();
	// Convert long int bytes to double equivalent; use same covrsion technique used for float 
	double *dvalue = (double*)(&lvalue);  
	return *dvalue;
}
