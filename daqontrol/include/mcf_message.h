/**
 * MCFMessage - Message class for CLB slow control
 */

#pragma once

#include <sys/time.h>
#include <util/elastic_interface.h>

#define MSG_TYPE_MASK 0x0FFF
#define MSG_TYPE_SHIFT 0
#define MSG_CLASS_MASK 0xF000
#define MSG_CLASS_SHIFT 12
	
#define MSG_CONTLEN_MASK 0xFFC0
#define MSG_CONTLEN_SHIFT 6
#define MSG_ID_MASK 0x003F
#define MSG_ID_SHIFT 0

/// Enumeration for the different classes of message
enum MCFClass
{
	error,
	command,
	reply,
	event
};

class MCFMessage {
public:
	/// MCFMessage Constructor
	MCFMessage() {};

	/// MCFMessage Constructor with initialisation
	MCFMessage(MCFClass mclass, int id, int type, std::vector<unsigned char> content); 

	~MCFMessage() {};

	/// Returns the length of the message content in bytes
	int getLength()
	{ 
		return content_.size();
	};

	/// Converts the message to a buffer of bytes
	void toByteBuffer(std::vector<unsigned char> &bb, long tOffset);

	/// Gets the message from a buffer of bytes
	void fromByteBuffer(std::vector<unsigned char> bb, long tOffset);

	MCFClass    class_;					///< Message class, command etc...
	int	       	type_;					///< Message type
	int	       	id_;					///< Message ID
	std::vector<unsigned char> content_;///< Data content of the message
	long		time_;					///< Time message was created
};