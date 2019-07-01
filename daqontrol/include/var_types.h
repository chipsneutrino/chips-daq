/**
 * Contains all msg type identifiers
 */

#pragma once

namespace VarTypes 
{

  enum VarType {
    U8(VarInfo.SCLTYP_U8,     "U8", short.class),
    U16(VarInfo.SCLTYP_U16,   "U16", int.class),
    U32(VarInfo.SCLTYP_U32,   "U32", long.class),
    U64(VarInfo.SCLTYP_U64,   "U64", BigInteger.class),
    I8(VarInfo.SCLTYP_I8,     "I8",byte.class),
    I16(VarInfo.SCLTYP_I16,   "I16",short.class),
    I32(VarInfo.SCLTYP_I32,   "I32",int.class),
    I64(VarInfo.SCLTYP_I64,   "I64",long.class),
    BOOL(VarInfo.SCLTYP_BOOL, "BOOL", boolean.class),
    F32(VarInfo.SCLTYP_F32,   "F32",float.class),
    F64(VarInfo.SCLTYP_F64,   "F64", double.class);

	static const int GROUP_SHIFT = 6; 
	
	static const int GROUP_DEBUG = 0x01;
	static const int MSG_DBG_BUS_RMW   = (GROUP_DEBUG << GROUP_SHIFT ) | 0x03;
	static const int MSG_DBG_I2C_READ  = (GROUP_DEBUG << GROUP_SHIFT ) | 0x04;
	static const int MSG_DBG_I2C_WRITE = (GROUP_DEBUG << GROUP_SHIFT ) | 0x05;
	static const int MSG_DBG_BUS_WRITE = (GROUP_DEBUG << GROUP_SHIFT ) | 0x02;
	static const int MSG_DBG_BUS_READ  = (GROUP_DEBUG << GROUP_SHIFT ) | 0x01;
	
	static const int GROUP_ACS = 0x06;

	static const int GROUP_CLB = 0x03;
	static const int MSG_CLB_SUB_VARS      = (GROUP_CLB << GROUP_SHIFT ) | 0x09;
	static const int MSG_CLB_EVENT         = (GROUP_CLB << GROUP_SHIFT ) | 0x02;
	static const int MSG_CLB_CLR_ERR_STATE = (GROUP_CLB << GROUP_SHIFT ) | 0x13;
	static const int EVT_CLB_UPDATE_VARS   = (GROUP_CLB << GROUP_SHIFT ) | 0x11;
	static const int MSG_CLB_GET_VARS      = (GROUP_CLB << GROUP_SHIFT ) | 0x07;
	static const int MSG_CLB_EXT_UPDATE    = (GROUP_CLB << GROUP_SHIFT ) | 0x12;
	static const int MSG_CLB_ACTIVATE_CFG  = (GROUP_CLB << GROUP_SHIFT ) | 0x05;
	static const int MSG_CLB_GET_STATE     = (GROUP_CLB << GROUP_SHIFT ) | 0x01;
	static const int MSG_CLB_UNSUB_VARS    = (GROUP_CLB << GROUP_SHIFT ) | 0x10;
	static const int MSG_CLB_SET_VARS      = (GROUP_CLB << GROUP_SHIFT ) | 0x08;
	static const int MSG_CLB_SUB_VARSRATE  = (GROUP_CLB << GROUP_SHIFT ) | 0x14;
	static const int EVT_CLB_STATE         = (GROUP_CLB << GROUP_SHIFT ) | 0x03;
	static const int MSG_CLB_STORE_CFG     = (GROUP_CLB << GROUP_SHIFT ) | 0x06;
	static const int EVT_CLB_UPDATE        = (GROUP_CLB << GROUP_SHIFT ) | 0x04;
	
	static const int GROUP_INS = 0x08;
	static const int MSG_INS_AHRS_SET_REG = (GROUP_INS << GROUP_SHIFT ) | 0x01;
	static const int MSG_INS_AHRS_GET_REG = (GROUP_INS << GROUP_SHIFT ) | 0x02;
	
	static const int GROUP_SYS = 0x02;
	static const int MSG_SYS_CONTUN_INIT   = (GROUP_SYS << GROUP_SHIFT ) | 0x10;
	static const int MSG_SYS_GOLDEN_UNLOCK = (GROUP_SYS << GROUP_SHIFT ) | 0x0B;
	static const int MSG_SYS_LOG_GET       = (GROUP_SYS << GROUP_SHIFT ) | 0x0C;
	static const int MSG_SYS_GOLDEN_STOP   = (GROUP_SYS << GROUP_SHIFT ) | 0x0A;
	static const int EVT_SYS_UPDATE        = (GROUP_SYS << GROUP_SHIFT ) | 0x06;
	static const int MSG_SYS_P_LOG_GET     = (GROUP_SYS << GROUP_SHIFT ) | 0x0D;
	static const int MSG_SYS_DATEREV       = (GROUP_SYS << GROUP_SHIFT ) | 0x02;
	static const int MSG_SYS_CONTUN_SEND   = (GROUP_SYS << GROUP_SHIFT ) | 0x11;
	static const int MSG_SYS_RT_CONFIG     = (GROUP_SYS << GROUP_SHIFT ) | 0x18;
	static const int MSG_SYS_VERIFY        = (GROUP_SYS << GROUP_SHIFT ) | 0x08;
	static const int MSG_SYS_UPDATE_START  = (GROUP_SYS << GROUP_SHIFT ) | 0x05;
	static const int MSG_SYS_PING          = (GROUP_SYS << GROUP_SHIFT ) | 0x01;
	static const int MSG_SYS_UPDATE        = (GROUP_SYS << GROUP_SHIFT ) | 0x06;
	static const int MSG_SYS_RESET         = (GROUP_SYS << GROUP_SHIFT ) | 0x04;
	static const int MSG_SYS_BOOT          = (GROUP_SYS << GROUP_SHIFT ) | 0x09;
	static const int EVT_SYS_CONTUN_RECV   = (GROUP_SYS << GROUP_SHIFT ) | 0x12;
	static const int MSG_SYS_UPDATE_END    = (GROUP_SYS << GROUP_SHIFT ) | 0x07;
	static const int MSG_SYS_IMG_GET       = (GROUP_SYS << GROUP_SHIFT ) | 0x0E;
	static const int MSG_SYS_EVT_TARGET    = (GROUP_SYS << GROUP_SHIFT ) | 0x03;
	
	static const int GROUP_NET = 0x04;
	static const int MSG_NET_MUX_DEST = (GROUP_NET << GROUP_SHIFT ) | 0x01;
	
	static const int GROUP_OPT = 0x05;
	
	static const int GROUP_BSE = 0x07;
	static const int MSG_BSE_RESET            = (GROUP_BSE << GROUP_SHIFT ) | 0x02;
	static const int MSG_BSE_CONFIGURE        = (GROUP_BSE << GROUP_SHIFT ) | 0x01;
	static const int MSG_BSE_BPS_DBG_CMDRPLY  = (GROUP_BSE << GROUP_SHIFT ) | 0x13;
	static const int MSG_BSE_EDFA_DBG_CMD     = (GROUP_BSE << GROUP_SHIFT ) | 0x11;
	static const int MSG_BSE_EDFA_DBG_CMDRPLY = (GROUP_BSE << GROUP_SHIFT ) | 0x12;
};
