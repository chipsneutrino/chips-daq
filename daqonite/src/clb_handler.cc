/**
 * CLBHandler - Handler class for the CLB data stream
 */

#include "clb_handler.h"

CLBHandler::CLBHandler(boost::asio::io_service* io_service, MonitoringGui *daqGui, 
					   DataHandler *data_handler, bool* mode) :
					   fDaq_gui(daqGui), fData_handler(data_handler), 
					   fMode(mode), fBuffer_size(buffer_size),
					   fSocket_optical(*io_service, udp::endpoint(udp::v4(), default_opto_port)),
					   fSocket_monitoring(*io_service, udp::endpoint(udp::v4(), default_moni_port)) {

	// Setup the sockets
	udp::socket::receive_buffer_size option_clb(33554432);
	fSocket_optical.set_option(option_clb);
	fSocket_monitoring.set_option(option_clb);
}

CLBHandler::~CLBHandler() {
	// Empty
}

void CLBHandler::workOpticalData() {
	if (*fMode == true) {
		fSocket_optical.async_receive(boost::asio::buffer(&fBuffer_optical[0], fBuffer_size),
								       boost::bind(&CLBHandler::handleOpticalData, this,
								       boost::asio::placeholders::error,
								       boost::asio::placeholders::bytes_transferred));
	}
}

void CLBHandler::workMonitoringData() {
	fSocket_monitoring.async_receive(boost::asio::buffer(&fBuffer_monitoring[0], fBuffer_size),
									  boost::bind(&CLBHandler::handleMonitoringData, this,
									  boost::asio::placeholders::error,
									  boost::asio::placeholders::bytes_transferred));
}

void CLBHandler::handleOpticalData(boost::system::error_code const& error, std::size_t size) {
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

		// Add an optical packet to the monitoring
		if (fDaq_gui != NULL) {
			fDaq_gui->addOpticalPacket((unsigned int)fData_handler->fPomId_opt_clb, (unsigned int)header_optical.udpSequenceNumber());
		}

		workOpticalData();
	} else {
		std::cout << "daqonite - Error: Optical async_receive!" << std::endl;
	}
}

void CLBHandler::handleMonitoringData(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		// Check the packet has atleast a CLB header in it
		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "daqonite - Error: Invalid monitoring packet size: " << size << std::endl;
			workMonitoringData();
			return;
		}

		// Check that the size of the packet is consistent with what we expect
		const ssize_t minimum_size = sizeof(CLBCommonHeader) + sizeof(int) * 31;
		if ((ssize_t)size <= minimum_size) { std::cout << "daqonite - Error: Incomplete monitoring packet!" << std::endl; }

		// Cast the beggining of the packet to the CLBCommonHeader
		CLBCommonHeader const
				& header_monitoring =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fBuffer_monitoring[0]));

		// Check the type of the packet is monitoring from the CLBCommonHeader
		std::pair<int, std::string> const& type = getType(header_monitoring);
		if (type.first != MONI) { throw std::runtime_error("daqonite - Error: Incorrect type not monitoring!"); }

		// Assign the variables we need from the header
		fData_handler->fPomId_mon_clb = header_monitoring.pomIdentifier();
		fData_handler->fTimestamp_s_mon_clb = header_monitoring.timeStamp().sec();

		// Get the monitoring hits data
		std::fill_n(fData_handler->fHits_mon_clb, 30, 0);
		for (int i = 0; i < 30; ++i) {
			const uint32_t
					* const field =
							static_cast<const uint32_t* const >
									(static_cast<const void* const >(&fBuffer_monitoring[0] + sizeof(CLBCommonHeader) + i * 4));
			fData_handler->fHits_mon_clb[i] = htonl(*field);
		}

		// Get the other monitoring info by casting into the SCData struct
		const SCData
				* const scData =
						static_cast<const SCData* const > (static_cast<const void* const > (&fBuffer_monitoring[0]
								+ minimum_size));


		fData_handler->fPad_mon_clb = scData->pad;
		fData_handler->fValid_mon_clb = ntohl(scData->valid);
		fData_handler->fTemperate_mon_clb = (uint16_t)ntohs(scData->temp) / (uint16_t)100.0;
		fData_handler->fHumidity_mon_clb = (uint16_t)ntohs(scData->humidity) / (uint16_t)100.0;

		if (fDaq_gui != NULL) {
			fDaq_gui->addMonitoringPacket((unsigned int)fData_handler->fPomId_mon_clb, fData_handler->fHits_mon_clb, 
			  							  fData_handler->fTemperate_mon_clb, fData_handler->fHumidity_mon_clb,
										  (unsigned int)header_monitoring.udpSequenceNumber());
		}

		if (*fMode == true) { fData_handler->fillMonCLBTree(); }

		workMonitoringData();
	} else {
		std::cout << "daqonite - Error: Monitoring async_receive!" << std::endl;
	}
}

std::pair<int, std::string> CLBHandler::getType(CLBCommonHeader const& header) {
	const static std::pair<int, std::string> unknown = std::make_pair(-1, "unknown");
	const static std::pair<int, std::string> acoustic = std::make_pair(ACOU, "acoustic data");
	const static std::pair<int, std::string> optical = std::make_pair(OPTO, "optical data");
	const static std::pair<int, std::string> monitoring = std::make_pair(MONI, "monitoring data");

	if (header.dataType() == tmch) { return monitoring; }
	else if (header.dataType() == ttdc) { return optical; }
	else if (header.dataType() == taes) { return acoustic; }
	return unknown;
}

