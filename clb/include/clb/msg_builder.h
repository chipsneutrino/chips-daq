/**
 * MsgBuilder - Builds km3net messages MCF + SRP protocols
 */

#pragma once

#include <sys/time.h>
#include <vector>
#include <util/elastic_interface.h>
#include <util/logging.h>

// Packet and SRP defines
#define SRP_SIZE            4
#define SRP_FLAG_MSG 		0x1
#define SRP_FLAG_DONTACK	0x2
#define SRP_FLAG_ACK_SHIFT	4
#define SRP_FLAG_ACK_MASK 	0xF0

#define MCFPACKET_MAX_SIZE  1400
#define TIME_MOD 			24L * 60 * 60 * 1000
#define HDR_SIZE 			6
#define MSG_HDR_SIZE 		6
#define MAX_CONTENT_SIZE    MCFPACKET_MAX_SIZE - (MSG_HDR_SIZE + HDR_SIZE);

// MCF defines
#define MSG_TYPE_MASK       0x0FFF
#define MSG_TYPE_SHIFT      0
#define MSG_CLASS_MASK      0xF000
#define MSG_CLASS_SHIFT     12
	
#define MSG_CONTLEN_MASK    0xFFC0
#define MSG_CONTLEN_SHIFT   6
#define MSG_ID_MASK         0x003F
#define MSG_ID_SHIFT        0

/// Enumeration for the different classes of message
enum MCFClass
{
	error,
	command,
	reply,
	event
};

class MsgBuilder: protected Logging {
public:
	/// MsgBuilder Constructor
	MsgBuilder();

	/// MsgBuilder Constructor with initialisation from types etc...
	MsgBuilder(MCFClass mclass, int id, int type, std::vector<unsigned char> content); 

	/// Converts the message to a vector of bytes
	std::vector<unsigned char> toBytes();

	/// Gets the message from a vector of bytes
	bool fromBytes(std::vector<unsigned char> bb);

	/// Returns the length of the message content in bytes
	int getLength()
	{ 
		return content_.size();
	};
		
	MCFClass    class_;					///< Message class, command etc...
	int	       	type_;					///< Message type
    int         packet_id_;             ///< MCF packet ID
	int	       	msg_id_;				///< MCF message ID
	std::vector<unsigned char> content_;///< Data content of the message
	long		time_;					///< Time message was created
};
