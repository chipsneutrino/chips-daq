/**
 * DAQ_clb_handler - Handler class for the CLB data stream
 */

#include "DAQ_clb_handler.h"

DAQ_clb_handler::DAQ_clb_handler(boost::asio::io_service* io_service, Monitoring_gui *daqGui, 
								 DAQ_data_handler *data_handler, bool* mode) :
						 		 fDaq_gui(daqGui), fData_handler(data_handler), 
								 fMode(mode), fBuffer_size(buffer_size) {

	// Add the CLB Optical socket to the IO service
	fSocket_optical = new udp::socket(*io_service, udp::endpoint(udp::v4(), default_opto_port));
	udp::socket::receive_buffer_size option_clb_opt(33554432);
	fSocket_optical->set_option(option_clb_opt);

	// Add the CLB monitoring port to the IO service
	fSocket_monitoring = new udp::socket(*io_service, udp::endpoint(udp::v4(), default_moni_port));
	udp::socket::receive_buffer_size option_clb_mon(33554432);
	fSocket_monitoring->set_option(option_clb_mon);
}

DAQ_clb_handler::~DAQ_clb_handler() {
	delete fSocket_optical;
	delete fSocket_monitoring;
}

void DAQ_clb_handler::workOpticalData() {
	if (*fMode == true) {
		fSocket_optical->async_receive(boost::asio::buffer(&fBuffer_optical[0], fBuffer_size),
								       boost::bind(&DAQ_clb_handler::handleOpticalData, this,
								       boost::asio::placeholders::error,
								       boost::asio::placeholders::bytes_transferred));
	}
}

void DAQ_clb_handler::workMonitoringData() {
	fSocket_monitoring->async_receive(boost::asio::buffer(&fBuffer_monitoring[0], fBuffer_size),
									  boost::bind(&DAQ_clb_handler::handleMonitoringData, this,
									  boost::asio::placeholders::error,
									  boost::asio::placeholders::bytes_transferred));
}

void DAQ_clb_handler::handleOpticalData(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		// Check the packet has atleast a CLB header in it
		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "DAQonite - Error: Invalid optical packet size: " << size << std::endl;
			workOpticalData();
			return;
		}

		// Check the size of the packet is consistent with CLBCommonHeader + some hits
		if (((size - sizeof(CLBCommonHeader)) % sizeof(hit_t))!=0) {throw std::runtime_error("DAQonite - Error: Bad optical packet!");}

		// Cast the beggining of the packet to the CLBCommonHeader
		CLBCommonHeader const
				& header_optical =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fBuffer_optical[0]));

		// Check the type of the packet is optical from the CLBCommonHeader
		std::pair<int, std::string> const& type = getType(header_optical);
		if (type.first != OPTO) { throw std::runtime_error("DAQonite - Error: Incorrect type not optical!"); }

		// Assign the variables we need from the header
		fData_handler->fPomId_opt_clb = header_optical.pomIdentifier();
		fData_handler->fTimestamp_s_opt_clb = header_optical.timeStamp().sec();
		uint32_t TimeStampNSTicks = header_optical.timeStamp().tics();

		// Find the number of hits this packet contains and loop over them all
		const unsigned int nhits = (size - sizeof(CLBCommonHeader)) / sizeof(hit_t);
		if (nhits) {
			for (int i = 0; i<(int)nhits; ++i) {
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
				uint32_t orderedTime = (((uint32_t)time1) << 24) + (((uint32_t)time2) << 16) + (((uint32_t)time3) << 8) + ((uint32_t)time4);
				fData_handler->fTimestamp_ns_opt_clb = (TimeStampNSTicks * 16) + orderedTime;

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
		std::cout << "DAQonite - Error: Optical async_receive!" << std::endl;
	}
}

void DAQ_clb_handler::handleMonitoringData(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		// Check the packet has atleast a CLB header in it
		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "DAQonite - Error: Invalid monitoring packet size: " << size << std::endl;
			workMonitoringData();
			return;
		}

		// Check that the size of the packet is consistent with what we expect
		const ssize_t minimum_size = sizeof(CLBCommonHeader) + sizeof(int) * 31;
		if ((ssize_t)size <= minimum_size) { std::cout << "DAQonite - Error: Incomplete monitoring packet!" << std::endl; }

		// Cast the beggining of the packet to the CLBCommonHeader
		CLBCommonHeader const
				& header_monitoring =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fBuffer_monitoring[0]));

		// Check the type of the packet is monitoring from the CLBCommonHeader
		std::pair<int, std::string> const& type = getType(header_monitoring);
		if (type.first != MONI) { throw std::runtime_error("DAQonite - Error: Incorrect type not monitoring!"); }

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
		std::cout << "DAQonite - Error: Monitoring async_receive!" << std::endl;
	}
}

std::pair<int, std::string> DAQ_clb_handler::getType(CLBCommonHeader const& header) {
	const static std::pair<int, std::string> unknown = std::make_pair(-1,
			"unknown");
	const static std::pair<int, std::string> acoustic = std::make_pair(ACOU,
			"acoustic data");
	const static std::pair<int, std::string> optical = std::make_pair(OPTO,
			"optical data");
	const static std::pair<int, std::string> monitoring = std::make_pair(MONI,
			"monitoring data");

	if (header.dataType() == tmch) {
		return monitoring;
	}

	if (header.dataType() == ttdc) {
		return optical;
	}

	if (header.dataType() == taes) {
		return acoustic;
	}

	return unknown;
}

void DAQ_clb_handler::printOpticalStats() {
	std::cout << "***** Optical Packet Stats ******" << std::endl;
}

void DAQ_clb_handler::printMonitoringStats() {
	std::cout << "**** Monitoring Packet Stats ****" << std::endl;
}

void DAQ_clb_handler::printHeader(CLBCommonHeader const& header) {
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
	} else {
		std::cout << '\n';
	}

	std::cout << "POMStatus 3:       " << header.pomStatus(3) << '\n'
			  << "POMStatus 4:       " << header.pomStatus(4) << std::endl;
}

void DAQ_clb_handler::printOpticalData(const char* const buffer, ssize_t buffer_size,
		int max_col) {
	const unsigned int nhits = (buffer_size - sizeof(CLBCommonHeader))
			/ sizeof(hit_t);

	std::cout << "Number of hits: " << nhits << '\n';

	if (nhits) {
		const int printing = 20 > nhits ? nhits : 20;
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

void DAQ_clb_handler::printMonitoringData(const char* const buffer, ssize_t buffer_size,
		int max_col) {
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

