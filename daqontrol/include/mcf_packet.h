/**
 * MCFPacket - Packet class for CLB slow control
 */

#pragma once

#include <util/elastic_interface.h>
#include "mcf_message.h"
#include <vector>

#define SRP_FLAG_MSG 		0x1
#define SRP_FLAG_DONTACK	0x2
#define SRP_FLAG_ACK_SHIFT	4
#define SRP_FLAG_ACK_MASK 	0xF0

#define MCFPACKET_MAX_SIZE  1400
#define TIME_MOD 			24L * 60 * 60 * 1000
#define HDR_SIZE 			6
#define MSG_HDR_SIZE 		6
#define MAX_CONTENT_SIZE    MCFPACKET_MAX_SIZE - (MSG_HDR_SIZE + HDR_SIZE);

class MCFPacket {
public:
	/// MCFPacket Constructor
	MCFPacket();

	/// MCFPacket Constructor with initialisation
	MCFPacket(std::vector<unsigned char> data);

	~MCFPacket() {};

	/// Converts the packet to a buffer of bytes
	std::vector<unsigned char> toBytes();

	/// Gets the packet from a buffer of bytes
	void fromBytes(std::vector<unsigned char> data);

	/// Returns the number of bytes free for payload data
	int free()
	{ 
		return ( MCFPACKET_MAX_SIZE - used_ ) - MSG_HDR_SIZE; 
	};	

	/// Adds a message to the packet
	void addMessage(MCFMessage message);

	/// Resets the packet
	void reset();

	/// Returns the number of messages in the packet
	int msgCount() {
		return messages_.size();
	}

	/// Gets a message from the packet
	MCFMessage getMessage(int num);
	
private:		
	std::vector<MCFMessage> messages_;	///< Vector of the MCFMessages in this packet
	int used_;							///< Number of bytes used in packet
	long toffset_;						///< Current maximum time offset
};
