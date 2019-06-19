/**
 * MCFPacket - Packet class for CLB slow control
 */

#include "mcf_packet.h"

MCFPacket::MCFPacket() 
{
  	reset();
}

MCFPacket::MCFPacket(std::vector<unsigned char> data) 
{
  	fromBytes(data);
}

std::vector<unsigned char> MCFPacket::toBytes()
{  
	if (messages_.size() == 0)  return std::vector<unsigned char>();

	std::vector<unsigned char> bb;

	// We insert the SRP protocol bytes here for now!!!
	bb.push_back(SRP_FLAG_MSG);
	bb.push_back(66);
	bb.push_back(0);
	bb.push_back(0);

	int tt = toffset_ % (long)TIME_MOD;
	bb.push_back( (unsigned char)((tt & 0xFF000000) >> 24) );
	bb.push_back( (unsigned char)((tt & 0x00FF0000) >> 16) );
	bb.push_back( (unsigned char)((tt & 0x0000FF00) >>  8) );
	bb.push_back( (unsigned char)((tt & 0x000000FF)      ) );
	bb.push_back( (unsigned char) 0);
	bb.push_back( (unsigned char)(messages_.size() - 1)    );

	std::vector<MCFMessage>::iterator msgit;

	for(msgit=messages_.begin(); msgit!=messages_.end(); msgit++){
		(*msgit).toByteBuffer(bb, toffset_);    // append message to byte buffer
	}

	return bb;
}

void MCFPacket::fromBytes(std::vector<unsigned char> data) 
{
	reset();
			
	if (data.size() < (HDR_SIZE + MSG_HDR_SIZE)) 
	{
		g_elastic.log(WARNING, "Data to small for even one message");
		return;
	}
			
	// We ignore the SRP protocol bytes here for now!!!
	std::vector<unsigned char> bb(&data[4], &data[data.size()]);
		
	struct timeval tp;
	gettimeofday(&tp, NULL);       
	toffset_ = ( (tp.tv_sec * 1000 + tp.tv_usec / 1000) / (long)TIME_MOD ) * (long)TIME_MOD +
		(int)( (0xFFFFFFFF &  bb[0]) <<24 | (0xFFFFFFFF & bb[1]) <<16 | (0xFFFFFFFF & bb[2]) <<8 | (0xFFFFFFFF & bb[3]) );
			
	int count = (0xFF & bb[5]) + 1;  /// length from element 5 
	std::cout << "MCFPacket::fromBytes count " << count << std::endl;

	// remove the packet header
	bb.erase(bb.begin(),bb.begin()+HDR_SIZE);

	try {
		for (int i = 0; i < count; ++i)
		{
			MCFMessage msg;
			msg.fromByteBuffer(bb, toffset_);  
			messages_.push_back(msg);
			bb.erase(bb.begin(),bb.begin()+msg.getLength());   // Remove entries after getting the message
		}
	} catch (const std::runtime_error& e) {
		throw std::runtime_error(" MCFPacket::fromBytes() Failed to parse Message");
	}
}

void MCFPacket::addMessage(MCFMessage message)
{
	if (messages_.size() == 256) 
	{
		g_elastic.log(FATAL, "Packet can not contain more messages");
		throw std::runtime_error("Packet can not contain more messages");
	}

	if ((used_ + message.getLength() + MSG_HDR_SIZE) > MCFPACKET_MAX_SIZE)
	{
		g_elastic.log(FATAL, "Message does not fit in packet");
		throw std::runtime_error("Message does not fit in packet");		
	}

	if (toffset_ == -1 || toffset_ > message.time_) {
		toffset_ = message.time_;
	}
	
	if ((message.time_ - toffset_) > 0xFFFF) {
		g_elastic.log(FATAL, "Message exceeds maximum packet timespan");
		throw std::runtime_error("Message exceeds maximum packet timespan");	
	}
	
	messages_.push_back(message);
	used_ += message.getLength() + MSG_HDR_SIZE;
}

void MCFPacket::reset() 
{
	used_ = HDR_SIZE;
	messages_.clear();
	toffset_ = -1;
}

MCFMessage MCFPacket::getMessage(int num)
{
	return messages_[num];
}
