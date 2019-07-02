/**
 * MsgProcessor - Processor for CLB messages
 */

#include "msg_processor.h"

MsgProcessor::MsgProcessor(unsigned long ip_address, std::shared_ptr<boost::asio::io_service> io_service) 
    : socket_(*io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)) 
{
    boost::asio::ip::udp::resolver resolver(*io_service);
    boost::asio::ip::address_v4 address(ip_address);
    boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), address.to_string(), std::to_string(DEFAULT_PORT));
    boost::asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
    endpoint_ = *iter;

    cmd_id_ = (int)(rand()*63);

    g_elastic.log(DEBUG, "Setup MsgProcessor for {}", address.to_string());
}

MsgReader MsgProcessor::processCommand(int type, MsgWriter mw)
{
    // Create the message
    MsgBuilder msg(command, cmdId(), type, mw.toBytes());

    // Log message to elasticsearch (DEBUG)
    g_elastic.log(DEBUG, "Processing command of class {}, type {}, ID {}", msg.class_, msg.type_, msg.packet_id_);

    boost::system::error_code err;
    auto sent = socket_.send_to(boost::asio::buffer(msg.toBytes()), endpoint_, 0, err);
    //g_elastic.log(DEBUG, "Sent message of size {}", sent); 

    // Receieve the response
    char recv_buf[MCFPACKET_MAX_SIZE];
    int size = socket_.receive_from(boost::asio::buffer(recv_buf), endpoint_);
    //g_elastic.log(DEBUG, "Receieved response of size {}", size);

    std::vector<unsigned char> recv_vec(&recv_buf[0], &recv_buf[size]);
    MsgBuilder response;
    if (!response.fromBytes(recv_vec)) 
    {
        // We need to send again
        g_elastic.log(WARNING, "Could not read response!");
    }

    if ((response.msg_id_ != msg.msg_id_)) // Check the response ID matches the sent message ID
    {
        // We need to send again
        g_elastic.log(WARNING, "Response ID does not match {}!", response.msg_id_);        
    }

    if (response.class_ != reply) // Check the response is a "reply"
    {
        // We need to send again
        g_elastic.log(WARNING, "Response is not a reply {}!", response.class_);        
    }

    if (response.type_ != msg.type_) // Check the response type matches the sent message type
    {
        // We need to send again
        g_elastic.log(WARNING, "Response is not of the same type {}!", response.type_);        
    }

    // We have a vlid response we need to send the CLB an "ack" to stop it resending
    sendAck(response.packet_id_);

    // Fill a message reader with the message content
    MsgReader mr(response.content_);
    return mr;
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