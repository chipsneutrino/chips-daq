/**
 * MsgProcessor - Processor for CLB messages
 * 
 * MsgProcessor is responsible for creating and decoding messages, and tracking response objects.
 */

#pragma once


#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "mcf_message.h"
#include "mcf_packet.h"
#include "msg_writer.h"
#include "msg_reader.h"
#include <util/elastic_interface.h>

/// Maximum time a command may take before its deemed 'lost'
#define MAX_COMMAND_TIME 10000
#define MAX_KEEP_ALIVE_SEC 10

/// Default CLBv2 slow-control port (56014)
#define DEFAULT_PORT 0xDACE

/// Buffer size for response message
#define BUFFERSIZE 10000

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
    ~MsgProcessor() {};

	int cmdId() 
	{
		cmd_id_ = ( cmd_id_ + 1 ) & 0x3F;
		return cmd_id_;
	}

	MsgReader processCommand(int type, MsgWriter mw);

	/// Flushes the message processor, sending all queued messages.
	void flush();

	/**
	 * Sets batch mode of the CLBControl on or off.
	 * 
	 * When set, all commands will be batched until:
	 * <ol>
	 * 	<li>A response is read (Response.get())</li>
	 *  <li>The maximum packet size has been reached</li>
	 *  <li>flush() is invoked</li>
	 * </ol>
	 * 
	 * By default batching mode is off.
	 * 
	 * @param batch		Set batching mode on or off.
	 */
	void setBatchMode(bool batch)
	{
		batch_ = batch;
		if (!batch_) {
			flush(); // when disabeling batch, send all.
		}
	}

	/**
	 * Returns weather or not batch-mode is enabled.
	 * 
	 * @return     {@code true} if enabled, {@code false} otherwise.
	 */
	bool isBatchMode() {
    	return batch_;
	}

	/// Closes this message processor and all related resources.
	void close();

private:

    boost::asio::ip::udp::socket socket_;                 	///< Socket to send CLB monitoring data to
    char buffer_[BUFFERSIZE] __attribute__((aligned(8))); 	///< CLB monitoring socket buffer
	boost::asio::ip::udp::endpoint endpoint_;				///< BOOST endpoint for the CLB

	MCFPacket tx_packet_;		///< MCF Packet used for stacking messages
	bool batch_;				///< Are we in batch mode?
	int cmd_id_;				///< Random command ID
	bool stop_;					///< Is comms stopped?
};

