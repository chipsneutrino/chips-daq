/**
 * CLBHandler - Handler class for the CLB data stream
 * 
 * This class deals with the specifics of the CLB data stream, unpacking
 * the UDP binary stream into the actual data and storing into .root
 * files. It also supplies the monitoring GUI with the data it needs.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef CLB_HANDLER_H_
#define CLB_HANDLER_H_

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "TTree.h"

#include "clb_header_structs.h"
#include "clb_data_structs.h"
#include "monitoring_gui.h"
#include "data_handler.h"

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

class CLBHandler {
	public:
		/// Create a CLBHandler
		CLBHandler(boost::asio::io_service* io_service, MonitoringGui *daqGui, 
				   DataHandler *data_handler, bool* mode);
					
		/// Destroy a CLBHandler
		~CLBHandler();

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

		/**
		 * Gets the data type from CLBCommonHeader.
		 * Given a CLBCommonHeader it reads to appropriate byte to find the data type.
		 * Either optical, acoustic or monitoring.
		 * 
		 * @param header CLBCommonHeader from input data packet
		 * @return size Number of bytes received
		 */
		std::pair<int, std::string> getType(CLBCommonHeader const& header);

		// CLBHandler settings/input
		bool 							fCollect_optical;					///< Should we collect optical data?
		bool 							fCollect_monitoring;				///< Should we collect monitoring data?
		MonitoringGui*					fDaq_gui;							///< Pointer to the MonitoringGui
		DataHandler*					fData_handler;						///< Pointer to the DataHandler
		bool* 							fMode;								///< false = Monitoring, True = Running
		std::size_t const 				fBuffer_size;						///< Size of the buffers

		// BOOST data collection
		boost::asio::ip::udp::socket 	fSocket_optical;					///< Optical data UDP socket
		char fBuffer_optical[buffer_size] __attribute__((aligned(8)));		///< Optical data buffer

		boost::asio::ip::udp::socket	fSocket_monitoring;					///< Monitoring data UDP socket
		char fBuffer_monitoring[buffer_size] __attribute__((aligned(8)));	///< Monitoring data buffer

		void printHeader(CLBCommonHeader const& header) {
			bool const valid = validTimeStamp(header);
			bool const trailer = isTrailer(header);

			std::string name("");

			std::cout << "DataType:          " << header.dataType() << '\n'
					<< "RunNumber:         " << header.runNumber() << '\n'
					<< "UDPSequenceNumber: " << header.udpSequenceNumber() << '\n'

					<< "Timestamp:\n" << "          Seconds: " << header.timeStamp().sec()
					<< '\n' << "          Tics:    " << header.timeStamp().tics()
					<< '\n' << "          " << UTCTime_h(header.timeStamp(), valid)
					<< '\n'

					<< "POMIdentifier:     " << header.pomIdentifier() << " (MAC: " << POMID_h(
						header.pomIdentifier()) << name << ')' << '\n'
					<< "POMStatus 1:       " << header.pomStatus(1) << '\n'
					<< "POMStatus 2:       " << header.pomStatus(2);

			if (trailer && header.dataType() == ttdc) {
				std::cout << " (trailer)\n";
			} else { std::cout << '\n'; }

			std::cout << "POMStatus 3:       " << header.pomStatus(3) << '\n'
					<< "POMStatus 4:       " << header.pomStatus(4) << std::endl;
		}

		void printOpticalData(const char* const buffer, ssize_t buffer_size, int max_col) {
			const unsigned int num_hits = (buffer_size - sizeof(CLBCommonHeader))
					/ sizeof(hit_t);

			std::cout << "Number of hits: " << num_hits << '\n';

			if (num_hits) {
				const int printing = 20 > num_hits ? num_hits : 20;
				const unsigned int n = max_col > 37 ? max_col / 37 : 1;

				for (int i = 0; i < printing; ++i) {
					const hit_t
							* const hit =
									static_cast<const hit_t* const > (static_cast<const void* const > (buffer
											+ sizeof(CLBCommonHeader) + i
											* sizeof(hit_t)));

					std::cout << "Hit" << std::setfill('0') << std::setw(2) << i
							<< ": " << *hit << ' ';

					if ((i + 1) % n == 0) {
						std::cout << '\n';
					} else {
						std::cout << "| ";
					}
				}
			}
			std::cout << '\n';
		}

		void printMonitoringData(const char* const buffer, ssize_t buffer_size, int max_col) {
			const unsigned int n = max_col > 14 ? max_col / 14 : 1;

			for (int i = 0; i < 31; ++i) {
				const uint32_t
						* const field =
								static_cast<const uint32_t* const > (static_cast<const void* const > (buffer
										+ sizeof(CLBCommonHeader) + i * 4));

				std::cout << "CH" << std::setfill('0') << std::setw(2) << i << ": "
						<< std::setfill(' ') << std::setw(6) << htonl(*field) << "  ";

				if ((i + 1) % n == 0) {
					std::cout << '\n';
				}
			}
			std::cout << '\n';

			const ssize_t minimum_size = sizeof(CLBCommonHeader) + sizeof(int) * 31;

			if (buffer_size > minimum_size) {
				std::cout << "SlowControl data:\n"
						<< *static_cast<const SCData* const > (static_cast<const void* const > (buffer
								+ minimum_size)) << '\n';
			}
		}
};

#endif
