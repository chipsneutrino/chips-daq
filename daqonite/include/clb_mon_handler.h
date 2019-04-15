/**
 * CLBMonHandler - Handler class for the CLB monitoring data stream
 * 
 * This class deals with the specifics of the CLB data stream, unpacking
 * the UDP binary stream into the actual data and storing for use in monitoring
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef CLB_MON_HANDLER_H_
#define CLB_MON_HANDLER_H_

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "TTree.h"

#include "clb_header_structs.h"
#include "clb_data_structs.h"
#include "monitoring_gui.h"
#include "data_handler.h"

/// Buffer size in bytes for monitoring data
const static size_t buffer_size_mon = 10000;

/// The default port for CLB UDP monitoring data
const static unsigned int default_moni_port = 56017;

using boost::asio::ip::udp;

class CLBMonHandler {
	public:
		/// Create a CLBMonHandler
		CLBMonHandler(boost::asio::io_service* io_service, 
					  MonitoringGui *daqGui,
					  DataHandler *dataHandler,
					  bool *mode);
					
		/// Destroy a CLBMonHandler
		~CLBMonHandler();

		/**
		 * IO_service monitoring data work function.
		 * Calls the async_receive() on the IO_service for the monitoring data stream.
		 */
		void workMonitoringData();

	private:
		/**
		 * Callback completion function for monitoring data async_receive().
		 * Handles the received monitoring data after the async_receive() has completed. 
		 * It fills the ROOT TTrees with the decoded data. It also forwards on monitoring
		 * data to the GUI for visualising.
		 * 
		 * @param error Error code from async_receive()
		 * @param size Number of bytes received
		 */
		void handleMonitoringData(boost::system::error_code const& error, std::size_t size);

		// CLBMonHandler settings/input
		bool 							fCollect_monitoring;					///< Should we collect monitoring data?
		MonitoringGui*					fDaq_gui;								///< Pointer to the MonitoringGui
		DataHandler*					fData_handler;							///< Pointer to the DataHandler
		bool*		 					fMode;									///< false = Monitoring, True = Running
		std::size_t const 				fBuffer_size;							///< Size of the buffers

		boost::asio::ip::udp::socket	fSocket_monitoring;						///< Monitoring data UDP socket
		char fBuffer_monitoring[buffer_size_mon] __attribute__((aligned(8)));	///< Monitoring data buffer
};

#endif
