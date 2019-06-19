/**
 * MsgProcessor - Processor for CLB messages
 */

#include "msg_processor.h"

MsgProcessor::MsgProcessor(std::string ip_address, std::shared_ptr<boost::asio::io_service> io_service) 
    : socket_(*io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)) 
{
    boost::asio::ip::udp::resolver resolver(*io_service);
    boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), ip_address, std::to_string(DEFAULT_PORT));
    boost::asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
    endpoint_ = *iter;

    g_elastic.log(DEBUG, "Endpoint with address {} and port {}", ip_address, std::to_string(DEFAULT_PORT));

    batch_ = false;
    cmd_id_ = (int)(rand()*63);
    stop_ = false;
}

void MsgProcessor::postEvent(int type, MsgWriter mw)
{
    MCFMessage msg(event, cmdId(), type, mw.toBytes());
        
    postMessage(msg);
}

void MsgProcessor::postCommand(int type, MsgWriter mw)
{
    if (stop_) return;
    
    //if (decoder == NULL) decoder = (MessageDecoder<T>) StdDecoders.VOID;
    
    MCFMessage msg(command, cmdId(), type, mw.toBytes());

    g_elastic.log(DEBUG, "Sending command of type {} with trxID {}", msg.type_, msg.id_);

    //Response<T> response = new Response<T>(decoder, this, msg.id, msg.type);
    //_cache.add(response);

    postMessage(msg);
    
    //return response;
}

void MsgProcessor::flush() 
{
    if (tx_packet_.msgCount() == 0) return;

    // Send the contents of the packet to the CLB
    boost::system::error_code err;
    auto sent = socket_.send_to(boost::asio::buffer(tx_packet_.toBytes()), endpoint_, 0, err);
    std::cout << "Sent Payload --- " << sent << "\n";     

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
    //_cache.close();
}

void MsgProcessor::postMessage(MCFMessage message)
{
    // If the packet is already too full then first flush()
    if (tx_packet_.free() < message.content_.size()) {
        flush();
    }

    // Add the message to the packet
    tx_packet_.addMessage(message);

    // If not in batch mode flush() the packet
    if (!batch_) flush();

    getResponse();
}

void MsgProcessor::getResponse()
{
    char recv_buf[100];
    int size = socket_.receive_from(boost::asio::buffer(recv_buf), endpoint_);  
    std::cout << "Recieved " << size << " bytes" << std::endl;
    //for (int i=0; i<size; i++) {
    //    std::cout << (int)recv_buf[i] << ",";
    //}
    //std::cout << std::endl;

    std::vector<unsigned char> recv_vec(&recv_buf[0], &recv_buf[size]);
    MCFPacket packet(recv_vec);
    MCFMessage message = packet.getMessage(0);
    std::cout << "Type: " << message.type_ << ", Class: " << message.class_ << std::endl;

    MsgReader mr(message.content_);
    long hwDateRev = mr.readU32();
	long swDateRev = mr.readU32();

    printf("Hardware Version: %08x\n", hwDateRev);
    printf("Software Version: %08x\n", swDateRev);
}

/*
void MsgProcessor::receiveData(SocketAddress sender, byte[] data)
{
    MCFPacket mcfPacket = new MCFPacket(data);
    
    Response<?> resp;
    EventContainer<?> ec;
    MessageContext context;
    
    for (MCFMessage msg : mcfPacket)
    {
        context = new MessageContext(sender, msg.time, msg.type);

        switch (msg.class)
        {
        case event:
            // events are no longer processed.
            if (_event.containsKey(msg.type)) 
            {
                ec = _event.get(msg.type);
                try {
                    g_elastic.log(DEBUG, "Received event of type {}", msg.type)

                    AsyncEventHandler<?> h = ec.createHandler(context, new MessageReader(msg.content));
                    _evtProc.invokeLater(h);
                } catch (Exception e) {
                    LOG.error("Event processor failed for type %03x failed: %s", msg.type, e.getMessage());
                    // e.printStackTrace();
                }
            } else {
                LOG.warning("Received event for unknown type %03x", msg.type);
            }
            break;
        case error:
        case reply:
            resp = _cache.byId(msg.id);
            if (resp != NULL) {
                LOG.debug("Received reply for message trxID %d", msg.id);
                resp.decodeMessage(context, msg);
                _cache.remove(resp);
            } else {
                LOG.warning("Received response for unknown message trxID %d", msg.id);
            }
            break;
        default:
            LOG.warning("Received response for unknown message class %s with trxID %d", msg.clazz, msg.id);
        }
        
    }
}
*/