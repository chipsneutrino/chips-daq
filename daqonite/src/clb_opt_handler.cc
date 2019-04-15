/**
 * CLBOptHandler - Handler class for the CLB optical data stream
 */

#include "clb_opt_handler.h"

CLBOptHandler::CLBOptHandler(boost::asio::io_service* io_service, 
					   		 DataHandler *data_handler, 
					   		 bool* mode) :
							 fData_handler(data_handler), 
					   		 fMode(mode), 
							 fBuffer_size(buffer_size_opt),
					   		 fSocket_optical(*io_service, udp::endpoint(udp::v4(), default_opto_port)) {

	// Setup the sockets
	udp::socket::receive_buffer_size option_clb(33554432);
	fSocket_optical.set_option(option_clb);
}

CLBOptHandler::~CLBOptHandler() {
	// Empty
}

void CLBOptHandler::workOpticalData() {
	if (*fMode == true) {
		fSocket_optical.async_receive(boost::asio::buffer(&fBuffer_optical[0], fBuffer_size),
								      boost::bind(&CLBOptHandler::handleOpticalData, this,
									  boost::asio::placeholders::error,
								      boost::asio::placeholders::bytes_transferred));
	}
}

void CLBOptHandler::handleOpticalData(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		// Check the packet has atleast a CLB header in it
		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "daqonite - Error: Invalid optical packet size: " << size << std::endl;
			workOpticalData();
			return;
		}

		// Check the size of the packet is consistent with CLBCommonHeader + some hits
		if (((size - sizeof(CLBCommonHeader)) % sizeof(hit_t))!=0) {throw std::runtime_error("daqonite - Error: Bad optical packet!");}

		// Cast the beggining of the packet to the CLBCommonHeader
		CLBCommonHeader const
				& header_optical =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fBuffer_optical[0]));

		// Check the type of the packet is optical from the CLBCommonHeader
		std::pair<int, std::string> const& type = getType(header_optical);
		if (type.first != OPTO) { throw std::runtime_error("daqonite - Error: Incorrect type not optical!"); }

		// Assign the variables we need from the header
		fData_handler->fPomId_opt_clb = header_optical.pomIdentifier();
		fData_handler->fTimestamp_s_opt_clb = header_optical.timeStamp().sec();
		uint32_t time_stamp_ns_ticks = header_optical.timeStamp().tics();

		// Find the number of hits this packet contains and loop over them all
		const unsigned int num_hits = (size - sizeof(CLBCommonHeader)) / sizeof(hit_t);
		if (num_hits) {
			for (int i = 0; i<(int)num_hits; ++i) {
				// Cast the hits individually to the hit_t struct
				const hit_t
						* const hit =
								static_cast<const hit_t* const > (static_cast<const void* const > (&fBuffer_optical[0]
										+ sizeof(CLBCommonHeader) + i * sizeof(hit_t)));

				// Assign the hit channel
				fData_handler->fChannel_opt_clb = hit->channel;

				uint8_t time1 = hit->timestamp1; uint8_t time2 = hit->timestamp2; 
				uint8_t time3 = hit->timestamp3; uint8_t time4 = hit->timestamp4;

				// Need to change the ordering of the bytes to get the correct hit time
				uint32_t ordered_time = (((uint32_t)time1) << 24) + (((uint32_t)time2) << 16) + (((uint32_t)time3) << 8) + ((uint32_t)time4);
				fData_handler->fTimestamp_ns_opt_clb = (time_stamp_ns_ticks * 16) + ordered_time;

				// Assign the hit TOT
				fData_handler->fTot_opt_clb = hit->ToT;

				if (*fMode == true) { fData_handler->fillOptCLBTree(); }
			}
		}

		workOpticalData();
	} else {
		std::cout << "daqonite - Error: Optical async_receive!" << std::endl;
	}
}
