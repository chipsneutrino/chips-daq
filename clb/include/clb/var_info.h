/**
 * VarInfo  - Info about CLB Variables
 */

#pragma once

#include <sys/time.h>
#include <util/elastic_interface.h>

#define  SUBSYS_MASK   0x70000000
#define  SUBSYS_SHIFT  28
#define  VARIDX_MASK   0x0FF00000
#define  VARIDX_SHIFT  20

#define  BASETYP_MASK  0x000C0000
#define  BASETYP_SHIFT 18
#define  SCLSIZE_MASK  0x00030000
#define  SCLSIZE_SHIFT 16

#define  SCLTYP_MASK   0x000F0000
#define  SCLTYP_SHIFT  16

#define  SCLTYP_U8   0x0
#define  SCLTYP_U16  0x1
#define  SCLTYP_U32  0x2
#define  SCLTYP_U64  0x3
#define  SCLTYP_I8   0x4
#define  SCLTYP_I16  0x5
#define  SCLTYP_I32  0x6
#define  SCLTYP_I64  0x7

#define  SCLTYP_BOOL  0x8
#define  SCLTYP_F32   0xA
#define  SCLTYP_F64   0xB

//#define  VIRTUAL BIT(15)
//#define  READABLE BIT(14)
//#define  WRITABLE BIT(13)
#define  ARRSIZE_MASK   0x00000FFF
#define  ARRSIZE_SHIFT  0


/// Enumeration for the different variable types
enum VarType {
     U8,
    U16,
    U32,
    U64,
     I8, 
     I16,
     I32,
     I64,
    BOOL,
     F32,
     F64
};

class VarInfo {
public:
	/// VarInfo Constructor
	VarInfo() {};

	/// VarInfo Constructor with initialisation
	VarInfo( int id);
	//  VarInfo( int id, std::string name);  // the name is superfluous  but  I leave it there for possible future uses


	~VarInfo() {};


	// Set ID (and all the parameters related to it)
	void SetID(int id);

	/// get Variable id
	int GetID() { return id_; };

	/// get Variable Type
	VarInfo GetVarType() { return type_; };

	/// get Variable Array Size
	int GetArraySize() { return array_size_; };

	
	/// Returns whether or not this variable is an array.
	bool isArray() {  return (array_size_ != 1);	};

	
	/// Returns the type as readable string.
	/*
	public String typeString() {
	  if (this.arraySize == 1) {
	    return scalarType.name();
	  } else {
	    return scalarType.name() + "[" + this.arraySize + "]";
	  }
	  }*/


	
	/// Returns the subsystem ID.	
	int subsys() { return ( id_ & SUBSYS_MASK ) >> SUBSYS_SHIFT; };
	
	
	///Returns the index of the variable within the subsystem. 
	int index()  { return ( id_ & VARIDX_MASK) >> VARIDX_SHIFT; };

	/*
	public boolean isReadable() {
	  return (id & READABLE) != 0;
	}

	public boolean isWritable() {
	  return (id & WRITABLE) != 0;
	}

	public boolean isVirtual() {
	  return (id & VIRTUAL) != 0;
	}
	*/	



	
	/** id for this variable */
	int id_;
	/** name for this variable */
	std::string name_;
	/** base type of this variable */
	VarType type_;
	/** array size of this variable, 1 if its not an array */
	int array_size_;

};
