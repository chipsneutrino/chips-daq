/**
 * VarInfo - Info about CLB Variables
 */

#include "var_info.h"
	


VarInfo::VarInfo( int id ){

  id_          = id;
  type_        = VarType((id & SCLTYP_MASK) >> SCLTYP_SHIFT);
  array_size_  = ((id & ARRSIZE_MASK) >> ARRSIZE_SHIFT) + 1;

}


