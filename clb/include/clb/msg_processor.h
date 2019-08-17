/**
 * MsgProcessor - Processor for CLB messages
 * 
 * MsgProcessor is responsible for creating and decoding messages, and tracking response objects.
 */

#pragma once

#include <iostream>
#include <vector>
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <clb/msg_builder.h>
#include <clb/msg_writer.h>
#include <clb/msg_reader.h>
#include <util/elastic_interface.h>

#define DEFAULT_PORT 0xDACE /// Default CLBv2 slow-control port (56014)
#define MAX_ATTEMPTS 3
#define READ_TIMEOUT 1
#define PROCESS_WAIT 500000

class MsgProcessor {
public:
	/**
	 * Creates a new MsgProcessor object for the given CLB. Message processors take care of
	 * delivering commands, receiving replies, processing events etc.
	 * For high level control you would usually not use this class.
	 * 
	 * @param ip_address      The ip address of the CLB to control
	 */
    MsgProcessor(unsigned long ip_address, std::shared_ptr<boost::asio::io_service> io_service);

    /// Destroy a MsgProcessor
    ~MsgProcessor() 
	{
		socket_.close();
	};

	bool processCommand(int type, MsgWriter &mw, MsgReader &mr);

private:

	int cmdId() 
	{
		cmd_id_ = ( cmd_id_ + 1 ) & 0x3F;
		return cmd_id_;
	}

	bool sendCommand(std::vector<unsigned char> msg);

	bool getResponse(std::vector<unsigned char> &resp);

	bool checkResponse(MsgBuilder &response, std::vector<unsigned char> &resp, MsgBuilder &msg);

	void sendAck(int id);

    boost::asio::ip::udp::socket socket_;                 	///< Socket to send CLB monitoring data to
	boost::asio::ip::udp::endpoint endpoint_;				///< BOOST endpoint for the CLB

	int cmd_id_;				///< Random command ID
};

