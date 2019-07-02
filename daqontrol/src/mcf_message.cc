/**
 * MCFMessage - Message class for CLB slow control
 */

#include "mcf_message.h"
	
MCFMessage::MCFMessage(MCFClass mclass, int id, int type, std::vector<unsigned char> content) 
{
	class_   = mclass;
	type_    = type;
	id_      = id;
	content_ = content;
	
	struct timeval tp;
	gettimeofday(&tp, NULL);       
	time_    = tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

void MCFMessage::toByteBuffer(std::vector<unsigned char> &bb, long tOffset) 
{
	int typeAndClass, lengthAndId;

	short tt = (short)(0xFFFF & (time_ - tOffset));
	bb.push_back((unsigned char) ( 0xFF & (tt >> 8) ));
	bb.push_back((unsigned char) ( 0xFF & tt        ));
	
	typeAndClass  = (type_ << MSG_TYPE_SHIFT) & MSG_TYPE_MASK;
	typeAndClass |= (class_ << MSG_CLASS_SHIFT) & MSG_CLASS_MASK;
	short tc =  (short) (0xFFFF & typeAndClass);
	bb.push_back((unsigned char) ( 0xFF & (tc >> 8) ));
	bb.push_back((unsigned char) ( 0xFF &  tc       ));
	
	lengthAndId  = ( content_.size() << MSG_CONTLEN_SHIFT ) & MSG_CONTLEN_MASK;
	lengthAndId |= ( id_ << MSG_ID_SHIFT ) & MSG_ID_MASK;
	short li = (short) (0xFFFF & lengthAndId);
	bb.push_back((unsigned char) ( 0xFF & (li >> 8) ));
	bb.push_back((unsigned char) ( 0xFF &  li       ));
	
	bb.reserve(bb.size() + content_.size() );
	bb.insert(bb.end(),    content_.begin(), content_.end());
}

void MCFMessage::fromByteBuffer(std::vector<unsigned char> bb, long tOffset)
{  
	int t, typeAndClass, lengthAndId;
	if (bb.size() < 6) throw std::runtime_error("Not enough bytes remaining for message");
	
	time_ = tOffset + (long)( 0xFFFF & ((bb[0]<<8) | bb[1]) );

	typeAndClass = 0xFFFF & ((bb[2]<<8) | bb[3]);
	type_ = ( typeAndClass & MSG_TYPE_MASK  ) >> MSG_TYPE_SHIFT;
	t    = ( typeAndClass & MSG_CLASS_MASK ) >> MSG_CLASS_SHIFT;

	if (t >= MCFClass::event) throw std::runtime_error("Invalid class identifier");  
	class_ = MCFClass(t);

	lengthAndId =  0xFFFF & ( (bb[4]<<8) | bb[5]);
	
	int len = ( lengthAndId & MSG_CONTLEN_MASK ) >> MSG_CONTLEN_SHIFT;
	id_ = ( lengthAndId & MSG_ID_MASK ) >> MSG_ID_SHIFT;

	// TODO: FIX READING THE LENGTH
	len = bb.size() - 6;
	
	if ( 6 + len > bb.size()) throw std::runtime_error("Invalid message length, bigger than packet");
	  
	content_.reserve(len);
	content_ = std::vector<unsigned char>(&bb[6], &bb[6+len]);
}

