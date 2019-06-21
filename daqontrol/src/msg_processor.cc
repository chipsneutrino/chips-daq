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

    batch_ = false;
    cmd_id_ = (int)(rand()*63);
    stop_ = false;
}

MsgReader MsgProcessor::processCommand(int type, MsgWriter mw)
{
    // Create the message
    MCFMessage msg(event, cmdId(), type, mw.toBytes());

    // Log message to elasticsearch (DEBUG)
    g_elastic.log(DEBUG, "Sending command of type {} with trxID {}", msg.type_, msg.id_);

    // Add the message to the packet
    tx_packet_.addMessage(msg);

    // TODO: Implement the ability to add multiple messages to the same packet
    // Will need to add asynchronous response mechanism
    // For now we flush() the packet for every single message    
    flush();

    // Receieve the response
    char recv_buf[100];
    int size = socket_.receive_from(boost::asio::buffer(recv_buf), endpoint_);
    g_elastic.log(DEBUG, "Receieved response of size {} with trxID {}", size);  

    std::vector<unsigned char> recv_vec(&recv_buf[0], &recv_buf[size]);
    MCFPacket packet(recv_vec);

    // Fill a message reader with the message content
    MsgReader mr(packet.getMessage(0).content_);
    return mr;
}

void MsgProcessor::flush() 
{
    if (tx_packet_.msgCount() == 0) return;

    // Send the contents of the packet to the CLB
    boost::system::error_code err;
    socket_.send_to(boost::asio::buffer(tx_packet_.toBytes()), endpoint_, 0, err);   

    tx_packet_.reset(); // Reset the packet (empty)
} 

void MsgProcessor::close()
{
    if (stop_) return;

    stop_ = true;
    if (tx_packet_.msgCount() > 0)
    {
        flush(); // flush() packet before closing socket
    }

    socket_.close();
}