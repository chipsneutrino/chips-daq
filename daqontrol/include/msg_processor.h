/**
 * MsgProcessor - Processor for CLB messages
 * 
 * MsgProcessor is responsible for creating and decoding messages, and tracking response objects.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
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
    MsgProcessor(std::string ip_address, std::shared_ptr<boost::asio::io_service> io_service);

    /// Destroy a MsgProcessor
    ~MsgProcessor() {};

	int cmdId() 
	{
		cmd_id_ = ( cmd_id_ + 1 ) & 0x3F;
		return cmd_id_;
	}

	/**
	 * Posts an event to a node or a broadcast group.
	 * Unlike commands, events do not require a response. The event may not be send immediately. 
	 * Use {@link #flush()} to send the message, or disable batch mode using 
	 * {@link #setBatchMode(boolean)}. Events may also be broadcasted, using a broadcast socket.
	 *
	 * @param type         The message type identifier.
	 * @param mw           The content of the event.
	 */
    void postEvent(int type, MsgWriter mw);

	/**
	 * Posts a command to a node.
	 * Commands are posted to a MessagePacket. If the processor is in batch mode, the command
	 * may not be send immediately.
	 * To force the sending of the command, invoke {@link #flush()}.
	 * 
	 * @param type         The type identifier of the command
	 * @param mw           The MsgWriter containing the command to send
	 * @param decoder      The Decoder to use for decoding, may be null for Void response.
	 * @return             A future response. 
	 */
	//void postCommand(int type, MsgWriter mw, MessageDecoder<T> decoder)
	void postCommand(int type, MsgWriter mw);

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

	void postMessage(MCFMessage message);

	void getResponse();

	//void receiveData(SocketAddress sender, byte[] data);

    // BOOST
    boost::asio::ip::udp::socket socket_;                 ///< Socket to send CLB monitoring data to
    char buffer_[BUFFERSIZE] __attribute__((aligned(8))); ///< CLB monitoring socket buffer
	boost::asio::ip::udp::endpoint endpoint_;	///< BOOST endpoint for the CLB

	MCFPacket tx_packet_;		///< MCF Packet used for stacking messages
	bool batch_;				///< Are we in batch mode?
	int cmd_id_;				///< Random command ID
	bool stop_;					///< Is comms stopped?
};

