/**
 * MsgProcessor - Processor for CLB messages
 */

#include <clb/msg_processor.h>

MsgProcessor::MsgProcessor(unsigned long ip_address, std::shared_ptr<boost::asio::io_service> io_service) 
    : socket_(*io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)) 
{
    boost::asio::ip::udp::resolver resolver(*io_service);
    boost::asio::ip::address_v4 address(ip_address);
    boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), address.to_string(), std::to_string(DEFAULT_PORT));
    boost::asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
    endpoint_ = *iter;

    // Set the timeout for the socket read
    struct timeval tv = {READ_TIMEOUT, 0};
    setsockopt(socket_.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    cmd_id_ = (int)(rand()*63);

    g_elastic.log(INFO, "Setup MsgProcessor for {}", address.to_string());
}

bool MsgProcessor::processCommand(int type, MsgWriter &mw, MsgReader &mr)
{
    // Create the message
    MsgBuilder msg(command, cmdId(), type, mw.toBytes());

    MsgBuilder response;
    for (int attempt=0; attempt<MAX_ATTEMPTS; attempt++) // Try up to MAX_ATTEMPTS for successful processing of command
    {
        // Send the command
        if(!sendCommand(msg.toBytes()))
        {
            if (attempt == (MAX_ATTEMPTS-1)) return false;
            continue;
        }

        std::vector<unsigned char> resp;
        if(!getResponse(resp))
        {
            if (attempt == (MAX_ATTEMPTS-1)) return false;
            continue;
        }

        // Check the response is valid
        if(!checkResponse(response, resp, msg))
        {
            sendAck(response.packet_id_); // Always want to send an ack
            if (attempt == (MAX_ATTEMPTS-1)) return false;
            continue;
        }    
        
        break;
    }

    // We have a valid response we need to send the CLB an "ack" to stop it resending
    sendAck(response.packet_id_);

    // Fill a message reader with the message content
    mr.fromBuffer(response.content_);

    // For now put a sleep in...
    sleep(1);
    
    return true;
}

bool MsgProcessor::sendCommand(std::vector<unsigned char> msg)
{
    boost::system::error_code err;
    int sent = socket_.send_to(boost::asio::buffer(msg), endpoint_, 0, err);

    if (sent != msg.size())
    {
        g_elastic.log(WARNING, "Only sent {} of {} bytes", sent, msg.size());   
        return false;
    }
    
    return true;
}

bool MsgProcessor::getResponse(std::vector<unsigned char> &resp) 
{
    // To make using a timeout easier we read from the underlying native socket
    unsigned char buff[MCFPACKET_MAX_SIZE];
    ssize_t size = read(socket_.native_handle(), buff, MCFPACKET_MAX_SIZE);

    if (size == 0)
    {
        g_elastic.log(DEBUG, "Did not receive any bytes");
        return false;
    } else if (size == -1) {
        g_elastic.log(DEBUG, "Got timeout");
        return false;        
    }

    resp.insert(resp.begin(), &buff[0], &buff[size]);
    return true;
}

bool MsgProcessor::checkResponse(MsgBuilder &response, std::vector<unsigned char> &resp, MsgBuilder &msg)
{
    if (!response.fromBytes(resp)) // Create the response message from the byte vector
    {
        g_elastic.log(WARNING, "Could not read response!");
        return false;
    }

    if ((response.msg_id_ != msg.msg_id_)) // Check the response ID matches the sent message ID
    {
        g_elastic.log(WARNING, "Response ID does not match {}!", response.msg_id_);   
        return false;     
    }

    if (response.class_ != reply) // Check the response is a "reply"
    {
        g_elastic.log(WARNING, "Response is not a reply {}!", response.class_);     
        return false;   
    }

    if (response.type_ != msg.type_) // Check the response type matches the sent message type
    {
        g_elastic.log(WARNING, "Response is not of the same type {}!", response.type_); 
        return false;       
    }

    return true;
}

void MsgProcessor::sendAck(int id) 
{
    // Create an empty vector of chars (ByteBuffer)
	std::vector<unsigned char> bb;

    bb.push_back(0);
    bb.push_back(0);
    bb.push_back((char)id); // We just add this single ack
    bb.push_back(0);

    // Set the flags for this single ack
    bb[0] |= ((SRP_FLAG_ACK_MASK & (1 << SRP_FLAG_ACK_SHIFT)));

    // Send the ack
    boost::system::error_code err;
    auto sent = socket_.send_to(boost::asio::buffer(bb), endpoint_, 0, err);
    //g_elastic.log(DEBUG, "Sent ack of size {}", sent); 
}