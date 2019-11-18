/**
 * MsgBuilder - Builds km3net messages MCF + SRP protocols
 */

#include <msg_builder.h>

MsgBuilder::MsgBuilder() 
{
  	content_.clear();
}

MsgBuilder::MsgBuilder(MCFClass mclass, int id, int type, std::vector<unsigned char> content)
{
	class_      = mclass;
	type_       = type;
    packet_id_  = id;
	msg_id_     = id;
	content_    = content;
	
	struct timeval tp;
	gettimeofday(&tp, NULL);
	time_       = tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

std::vector<unsigned char> MsgBuilder::toBytes()
{  
    // Create an empty vector of chars (ByteBuffer)
	std::vector<unsigned char> bb;

    // First 4 bytes are SRP protocol stuff
	bb.push_back(SRP_FLAG_MSG);
	bb.push_back(packet_id_); // Packet ID
	bb.push_back(0);
	bb.push_back(0);

    // Next 6 bytes are MCF packet stuff (we only have single message packets)
	int pt = time_ % (long)TIME_MOD; // First the packet time in 4 bytes
	bb.push_back( (unsigned char)((pt & 0xFF000000) >> 24) );
	bb.push_back( (unsigned char)((pt & 0x00FF0000) >> 16) );
	bb.push_back( (unsigned char)((pt & 0x0000FF00) >>  8) );
	bb.push_back( (unsigned char)((pt & 0x000000FF)      ) );
	bb.push_back( (unsigned char) 0); // Empty byte
	bb.push_back( (unsigned char) 0); // Number of messages in packet -1 (1-1=0)

    // Next 6 bytes is the actual MCF message header
	short mt = 0; // Time offset of message in packet in 2 bytes, for us always zero
	bb.push_back((unsigned char) ( 0xFF & (mt >> 8) ));
	bb.push_back((unsigned char) ( 0xFF & mt        ));
    int typeAndClass; // Type and class of the message in 2 bytes
	typeAndClass  = (type_ << MSG_TYPE_SHIFT) & MSG_TYPE_MASK;
	typeAndClass |= (class_ << MSG_CLASS_SHIFT) & MSG_CLASS_MASK;
	short tc =  (short) (0xFFFF & typeAndClass);
	bb.push_back((unsigned char) ( 0xFF & (tc >> 8) ));
	bb.push_back((unsigned char) ( 0xFF &  tc       ));
    int lengthAndId; // Payload length and ID of message in 2 bytes
	lengthAndId  = ( content_.size() << MSG_CONTLEN_SHIFT ) & MSG_CONTLEN_MASK;
	lengthAndId |= ( msg_id_ << MSG_ID_SHIFT ) & MSG_ID_MASK;
	short li = (short) (0xFFFF & lengthAndId);
	bb.push_back((unsigned char) ( 0xFF & (li >> 8) ));
	bb.push_back((unsigned char) ( 0xFF &  li       ));
	
    // Add the message payload
	bb.reserve(bb.size() + content_.size() );
	bb.insert(bb.end(),    content_.begin(), content_.end());

    // Return the filled vector of chars
	return bb;
}

bool MsgBuilder::fromBytes(std::vector<unsigned char> bb) 
{		
    // First check the received message is big enough	
	if (bb.size() < (SRP_SIZE + HDR_SIZE + MSG_HDR_SIZE)) 
	{
		//g_elastic.log(WARNING, "MsgBuilder: Received data too small, got {}", bb.size());
		return false;
	}

    // First 4 bytes are SRP protocol stuff
    int flags = 0xFF & bb[0]; // Get the SRP flags
    // int c = ((flags & SRP_FLAG_ACK_MASK) >> SRP_FLAG_ACK_SHIFT); // This is number of acks attached to message
    if ((flags & SRP_FLAG_MSG) == 0) // Check that there is an SRP_FLAG_MSG is present
    {
        g_elastic.log(WARNING, "MsgBuilder: No SRP_FLAG_MSG flag in response");
        return false;
    }
    packet_id_ = bb[1];

    // Next 6 bytes are MCF packet stuff (we only have single message packets)
	struct timeval tp;
	gettimeofday(&tp, NULL);       
	time_ = ( (tp.tv_sec * 1000 + tp.tv_usec / 1000) / (long)TIME_MOD ) * (long)TIME_MOD +
		(int)( (0xFFFFFFFF &  bb[4]) << 24 | (0xFFFFFFFF & bb[5]) << 16 | (0xFFFFFFFF & bb[6]) << 8 | (0xFFFFFFFF & bb[7]) );
    int count = (0xFF & bb[9]) + 1;  // number of messgages from element 5 in packet header 
    if (count != 1)
    {
        g_elastic.log(WARNING, "MsgBuilder: {} messages in the packet", count);
        return false;        
    }  

    // Next 6 bytes is the actual MCF message header		
    //long tOffset = (long)( 0xFFFF & ((bb[10]<<8) | bb[11]) ) - time_;
    //if (tOffset != 0)
    //{
    //    g_elastic.log(WARNING, "tOffset is not zero, it's {}", tOffset);
    //    return false;        
    //}

    int t, typeAndClass;
	typeAndClass = 0xFFFF & ((bb[12]<<8) | bb[13]);
	type_ = ( typeAndClass & MSG_TYPE_MASK  ) >> MSG_TYPE_SHIFT;
	t    = ( typeAndClass & MSG_CLASS_MASK ) >> MSG_CLASS_SHIFT;
	class_ = MCFClass(t);

    int lengthAndId;
	lengthAndId =  0xFFFF & ( (bb[14]<<8) | bb[15]);
	int len = ( lengthAndId & MSG_CONTLEN_MASK ) >> MSG_CONTLEN_SHIFT;
	msg_id_ = ( lengthAndId & MSG_ID_MASK ) >> MSG_ID_SHIFT;

	// TODO: FIX READING THE LENGTH
	len = bb.size() - SRP_SIZE - HDR_SIZE - MSG_HDR_SIZE;

    if (len < 0)
    {
        g_elastic.log(WARNING, "Invalid message payload length {}", len);
        return false;        
    }    
	  
	content_.reserve(len);
	content_ = std::vector<unsigned char>(&bb[16], &bb[16+len]);

    return true;
}