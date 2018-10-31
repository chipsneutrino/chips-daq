/**
 * DAQ_clb_handler - Handler class for the CLB data stream
 * 
 * This class deals with the specifics of the CLB data stream, unpacking
 * the UDP binary stream into the actual data and storing into .root
 * files. It also supplies the monitoring GUI with the data it needs.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef DAQ_CLB_HANDLER_H_
#define DAQ_CLB_HANDLER_H_

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "TTree.h"

#include "DAQ_clb_header_structs.h"
#include "DAQ_clb_data_structs.h"
#include "Monitoring_gui.h"

/// Buffer size in bytes for optical and monitoring data
const static size_t buffer_size = 10000;

/// The default port for CLB UDP optical data
const static unsigned int default_opto_port = 56015;

/// The default port for CLB UDP monitoring data
const static unsigned int default_moni_port = 56017;

/// Rate at which number of packets received is printed to stdout
#define TERMINALPRINTRATE 50000

#define MONI 0x1
#define ACOU 0x2
#define OPTO 0x4
#define AUTO (MONI | ACOU | OPTO)

const static unsigned int ttdc = 1414808643;
const static unsigned int taes = 1413563731;
const static unsigned int tmch = 1414349640;

using boost::asio::ip::udp;

class DAQ_clb_handler {
	public:

		/// Create a DAQ_clb_handler
		DAQ_clb_handler(boost::asio::io_service* io_service, bool mine_opt, bool mine_mon,
						Monitoring_gui *daqGui, bool* mode);
					
		/// Destroy a DAQ_clb_handler
		~DAQ_clb_handler();

		/** 
		 * Sets the pointers to the output file TTree's.
		 * Called from the DAQ_handler, which deals with the file IO, in order to set
		 * the TTree pointers so Fill() can be called from here.
		 * 
		 * @param output_tree_opt Pointer to optical TTree
		 * @param output_tree_mon Pointer to monitoring TTree
		 */
		void setSaveTrees(TTree * output_tree_opt, TTree * output_tree_mon);

		///Sets the pointers to the output file TTree's to NULL in the clb_handler
		void clearSaveTrees();

		/**
		 * IO_service optical data work function.
		 * Calls the async_receive() on the IO_service for the optical data stream.
		 */
		void workOpticalData();

		/**
		 * IO_service monitoring data work function.
		 * Calls the async_receive() on the IO_service for the monitoring data stream.
		 */
		void workMonitoringData();

	private:
	
		/**
		 * Callback completion function for optical data async_receive().
		 * Handles the received optical data after the async_receive() has completed. 
		 * It fills the ROOT TTrees with the decoded data.
		 * 
		 * @param error Error code from async_receive()
		 * @param size Number of bytes received
		 */
		void handleOpticalData(boost::system::error_code const& error, std::size_t size);

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

		/// Add the neccesary branches to the optical TTree
		void addOptTreeBranches();
		/// Add the neccesary branches to the monitoring TTree
		void addMonTreeBranches();

		/**
		 * Gets the data type from CLBCommonHeader.
		 * Given a CLBCommonHeader it reads to appropriate byte to find the data type.
		 * Either optical, acoustic or monitoring.
		 * 
		 * @param header CLBCommonHeader from input data packet
		 * @return size Number of bytes received
		 */
		std::pair<int, std::string> getType(CLBCommonHeader const& header);

		/// Print CLBCommonHeader to stdout
		void printHeader(CLBCommonHeader const& header);
		/// Print Optical data to stdout
		void printOpticalData(const char* const buffer, ssize_t buffer_size, int max_col);
		/// Print monitoring data to stdout
		void printMonitoringData(const char* const buffer, ssize_t buffer_size, int max_col);

		// DAQ_clb_handler settings/input
		bool 							fCollect_optical;					///< Should we collect optical data?
		bool 							fCollect_monitoring;				///< Should we collect monitoring data?
		Monitoring_gui*					fDaq_gui;							///< Pointer to the monitoring GUI
		bool* 							fMode;								///< false = Monitoring, True = Running
		std::size_t const 				fBuffer_size;						///< Size of the buffers

		// BOOST data collection
		boost::asio::ip::udp::socket* 	fSocket_optical;					///< Optical data UDP socket
		char fBuffer_optical[buffer_size] __attribute__((aligned(8)));		///< Optical data buffer
		

		boost::asio::ip::udp::socket*	fSocket_monitoring;					///< Monitoring data UDP socket
		char fBuffer_monitoring[buffer_size] __attribute__((aligned(8)));	///< Monitoring data buffer
	
		// Output
		TTree* 							fOutput_tree_optical;				///< Pointer to the optical TTree
		TTree* 							fOutput_tree_monitoring;			///< Pointer to the monitoring TTree

		UInt_t 							fPomId_optical;						///< Optical Data: Header POM ID
		UChar_t 						fChannel_optical;					///< Optical Data: Hit Channel ID
		UInt_t 							fTimestamp_s_optical;				///< Optical Data: Header timestamp [s]
		UInt_t 							fTimestamp_w_optical;				///< Optical Data: Header ticks
		UInt_t 							fTimestamp_ns_optical;				///< Optical Data: Hit timestamp [ns]
		Char_t 							fTot_optical;						///< Optical Data: Hit TOT value
		
		UInt_t 							fPomId_monitoring;					///< Monitoring Data: Header POM ID
		UInt_t 							fTimestamp_s_monitoring;			///< Monitoring Data: Header timestamp [s]
		UInt_t 							fPad_monitoring;					///< Monitoring Data: Header Pad
		UInt_t 							fValid_monitoring;					///< Monitoring Data: Header Valid
		float 							fTemperate_monitoring;				///< Monitoring Data: Temperature data
		float 							fHumidity_monitoring;				///< Monitoring Data: Humidity data
		unsigned int 					fMonitoringHits[30];				///< Monitoring Data: # Hits on channels

		// Packet counters
		int 							fCounter_optical;					///< Optical packet counter
		int 							fCounter_monitoring;				///< Monitoring packet counter
};

#endif
