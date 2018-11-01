/**
 * DAQ_clb_handler - Handler class for the CLB data stream
 */

#include "DAQ_clb_handler.h"

DAQ_clb_handler::DAQ_clb_handler(boost::asio::io_service* io_service, bool mine_opt, bool mine_mon,
								 Monitoring_gui *daqGui, bool* mode) :
						 		 fCollect_optical(mine_opt), fCollect_monitoring(mine_mon),
						 		 fDaq_gui(daqGui), fMode(mode), fBuffer_size(buffer_size) {

	// Add the CLB Optical socket to the IO service
	fSocket_optical = new udp::socket(*io_service, udp::endpoint(udp::v4(), default_opto_port));
	udp::socket::receive_buffer_size option_clb_opt(33554432);
	fSocket_optical->set_option(option_clb_opt);

	// Add the CLB monitoring port to the IO service
	fSocket_monitoring = new udp::socket(*io_service, udp::endpoint(udp::v4(), default_moni_port));
	udp::socket::receive_buffer_size option_clb_mon(33554432);
	fSocket_monitoring->set_option(option_clb_mon);

	// Output TTree's
	fOutput_tree_optical = NULL;
	fOutput_tree_monitoring = NULL;

	// Output Variables Optical
	fPomId_optical 			= 0;
	fTimestamp_s_optical	= 0;
	fTimestamp_w_optical 	= 0;
	fTimestamp_ns_optical	= 0;

	// Output Variables Monitoring
	fPomId_monitoring 		= 0;
	fTimestamp_s_monitoring = 0;
	fPad_monitoring 		= 0;
	fValid_monitoring 		= 0;
	fTemperate_monitoring 	= 0.0;
	fHumidity_monitoring 	= 0.0;
}

DAQ_clb_handler::~DAQ_clb_handler() {
	// Empty
}

void DAQ_clb_handler::setSaveTrees(TTree * output_tree_opt, TTree * output_tree_mon) {
	// Check to see if the TTree's are not NULL...
	if (fCollect_optical && output_tree_opt == NULL) {
		throw std::runtime_error("DAQonite - Error: Setting NULL optical tree!");
	}

	if (fCollect_monitoring && output_tree_mon == NULL) {
		throw std::runtime_error("DAQonite - Error: Setting NULL monitoring tree!");
	}

	// Check to see if we already have TTree's
	if (fCollect_optical && fOutput_tree_optical != NULL) {
		std::cout << "DAQonite - Error: Optical tree is not NULL!" << std::endl;
		clearSaveTrees();
	}

	if (fCollect_monitoring && fOutput_tree_monitoring != NULL) {
		std::cout << "DAQonite - Error: Monitoring tree is not NULL!" << std::endl;
		clearSaveTrees();
	}

	// Set the TTree's
	fOutput_tree_optical = output_tree_opt;
	fOutput_tree_monitoring = output_tree_mon;

	// Add the branches to the TTree
	if (fCollect_optical && fOutput_tree_optical != NULL) { addOptTreeBranches(); }
	if (fCollect_monitoring && fOutput_tree_monitoring != NULL) { addMonTreeBranches(); }
}

void DAQ_clb_handler::clearSaveTrees() {
	// Set the TTree's to NULL
	fOutput_tree_optical = NULL;
	fOutput_tree_monitoring = NULL;
}

void DAQ_clb_handler::workOpticalData() {
	if (fCollect_optical && *fMode == true) {
		fSocket_optical->async_receive(boost::asio::buffer(&fBuffer_optical[0], fBuffer_size),
								   boost::bind(&DAQ_clb_handler::handleOpticalData, this,
								   boost::asio::placeholders::error,
								   boost::asio::placeholders::bytes_transferred));
	}
}

void DAQ_clb_handler::workMonitoringData() {
	if (fCollect_monitoring) {
		fSocket_monitoring->async_receive(boost::asio::buffer(&fBuffer_monitoring[0], fBuffer_size),
								   boost::bind(&DAQ_clb_handler::handleMonitoringData, this,
								   boost::asio::placeholders::error,
								   boost::asio::placeholders::bytes_transferred));
	}
}

void DAQ_clb_handler::addOptTreeBranches() {
	fOutput_tree_optical->Branch("PomId", &fPomId_optical, "fPomId_optical/i");
	fOutput_tree_optical->Branch("Channel", &fChannel_optical, "fChannel_optical/b");
	fOutput_tree_optical->Branch("TimeStamp_s", &fTimestamp_s_optical, "fTimestamp_s_optical/i");
	fOutput_tree_optical->Branch("TimeStamp_ns", &fTimestamp_ns_optical, "fTimestamp_ns_optical/i");
	fOutput_tree_optical->Branch("ToT", &fTot_optical, "fTot_optical/B");
}

void DAQ_clb_handler::addMonTreeBranches() {
	fOutput_tree_monitoring->Branch("PomId", &fPomId_monitoring, "fPomId_monitoring/i");
	fOutput_tree_monitoring->Branch("TimeStamp_s", &fTimestamp_s_monitoring, "fTimestamp_s_monitoring/i");
	fOutput_tree_monitoring->Branch("Pad", &fPad_monitoring, "fPad_monitoring/i");
	fOutput_tree_monitoring->Branch("Valid", &fValid_monitoring, "fValid_monitoring/i");
	fOutput_tree_monitoring->Branch("Temperate", &fTemperate_monitoring, "fTemperate_monitoring/F");
	fOutput_tree_monitoring->Branch("Humidity", &fHumidity_monitoring, "fHumidity_monitoring/F");

	// If we are not collecting the optical data hits, I will save the monitoring hits to file
	if (!fCollect_optical) {
		fOutput_tree_monitoring->Branch("Hits",&fMonitoring_hits,"fMonitoring_hits[30]/I");	
	}
}

void DAQ_clb_handler::handleOpticalData(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		if (fBuffer_size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "DAQonite - Error: Invalid buffer size OPTO: " << fBuffer_size << std::endl;
			workOpticalData();
			return;
		}

		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "DAQonite - Error: Invalid packet size OPTO: " << size << std::endl;
			workOpticalData();
			return;
		}

		CLBCommonHeader const
				& header_optical =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fBuffer_optical[0]));

		// Check the type...
		std::pair<int, std::string> const& type = getType(header_optical);
		if (type.first != OPTO) { throw std::runtime_error("DAQonite - Error: Incorrect type not OPTO!"); }

		fPomId_optical = (UInt_t) header_optical.pomIdentifier();
		fTimestamp_s_optical = (UInt_t) header_optical.timeStamp().sec();

		UInt_t TimeStampNSTicks = (UInt_t)header_optical.timeStamp().tics();
		fTimestamp_w_optical = TimeStampNSTicks*16;

		if (((size - sizeof(CLBCommonHeader)) % sizeof(hit_t))!=0) {throw std::runtime_error("DAQonite - Error: Bad packet OPTO!");}
		const unsigned int nhits = (size - sizeof(CLBCommonHeader)) / sizeof(hit_t);

		if (nhits) {
			for (int i = 0; i < (int)nhits; ++i) {
				const hit_t
						* const hit =
								static_cast<const hit_t* const > (static_cast<const void* const > (&fBuffer_optical[0]
										+ sizeof(CLBCommonHeader) + i * sizeof(hit_t)));

				fChannel_optical = (UChar_t)hit->channel;

				uint8_t time1 = hit->timestamp1; uint8_t time2 = hit->timestamp2; 
				uint8_t time3 = hit->timestamp3; uint8_t time4 = hit->timestamp4;

				// Need to change the ordering of the bytes to get the correct hit time
				uint32_t orderedTime = (((uint32_t)time1) << 24) + (((uint32_t)time2) << 16) + (((uint32_t)time3) << 8) + ((uint32_t)time4);
				fTimestamp_ns_optical = (TimeStampNSTicks * 16) + (UInt_t)orderedTime;

				fTot_optical = (UChar_t)hit->ToT;

				if (*fMode == true && fOutput_tree_optical!= NULL) { fOutput_tree_optical->Fill(); }
			}
		}

		// Add an optical packet to the monitoring
		if (fDaq_gui != NULL) {
			fDaq_gui->addOpticalPacket((unsigned int)fPomId_optical, (unsigned int)header_optical.udpSequenceNumber());
		}

		workOpticalData();
	}
}

void DAQ_clb_handler::handleMonitoringData(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		if (fBuffer_size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "DAQonite - Error: Invalid buffer size MONI: " << fBuffer_size << std::endl;
			workMonitoringData();
			return;
		}

		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "DAQonite - Error: Invalid packet size MONI: " << size << std::endl;
			workMonitoringData();
			return;
		}

		CLBCommonHeader const
				& header_monitoring =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fBuffer_monitoring[0]));

		// Check the type...
		std::pair<int, std::string> const& type = getType(header_monitoring);
		if (type.first != MONI) { throw std::runtime_error("DAQonite - Error: Incorrect type not MONI!"); }

		// Get what we need from the header
		fPomId_monitoring = (UInt_t) header_monitoring.pomIdentifier();
		fTimestamp_s_monitoring = (UInt_t) header_monitoring.timeStamp().sec();

		// Get the monitoring hits data
		std::fill_n(fMonitoring_hits, 30, 0);
		for (int i = 0; i < 30; ++i) {
			const uint32_t
					* const field =
							static_cast<const uint32_t* const >
									(static_cast<const void* const >(&fBuffer_monitoring[0] + sizeof(CLBCommonHeader) + i * 4));
			fMonitoring_hits[i] = (unsigned int)htonl(*field);
		}

		// Get the other monitoring info
		const ssize_t minimum_size = sizeof(CLBCommonHeader) + sizeof(int) * 31;
		if (((ssize_t)size > minimum_size)) {
			const SCData
					* const scData =
							static_cast<const SCData* const > (static_cast<const void* const > (&fBuffer_monitoring[0]
									+ minimum_size));

			fPad_monitoring = (UInt_t)scData->pad;
			fValid_monitoring = ntohl(scData->valid);
			fTemperate_monitoring = (float)ntohs(scData->temp) / (float)100;
			fHumidity_monitoring = (float)ntohs(scData->humidity) / (float)100;

			if (fDaq_gui != NULL) {
				fDaq_gui->addMonitoringPacket((unsigned int)fPomId_monitoring,
				 							  (unsigned int)header_monitoring.timeStamp().inMilliSeconds(),
											  fMonitoring_hits, fTemperate_monitoring, fHumidity_monitoring,
											  (unsigned int)header_monitoring.udpSequenceNumber());
			}

			if (*fMode == true && fOutput_tree_monitoring!= NULL) { fOutput_tree_monitoring->Fill(); }
		} else {
			// Do not fill anything is we get here!
			std::cout << "DAQonite - Error: Incomplete monitoring packet!" << std::endl;
		}

		workMonitoringData();
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

