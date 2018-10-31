/**
 * DAQ_clb_handler - Handler class for the CLB data stream
 */

#include "DAQ_clb_handler.h"

DAQ_clb_handler::DAQ_clb_handler(boost::asio::ip::udp::socket* socket_opt, bool collect_opt,
						 		 boost::asio::ip::udp::socket* socket_mon, bool collect_mon,
						 		 std::size_t buffer_size, Monitoring_gui *daqGui, bool* running) :
						 		 fSocket_optical(socket_opt), fCollect_optical(collect_opt),
						 		 fSocket_monitoring(socket_mon), fCollect_monitoring(collect_mon),
						 		 fBuffer_size(buffer_size), fDaq_gui(daqGui) {

	// Add the running bool pointer...
	fRunning = running;
	
	fSave_data = false;

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

	// Packet Counters
	fCounter_optical 		= 0;
	fCounter_monitoring 	= 0;
}

DAQ_clb_handler::~DAQ_clb_handler() {
	// Empty
}

void DAQ_clb_handler::setSaveTrees(bool saveData, TTree * output_tree_opt, TTree * output_tree_mon) {
	fSave_data = saveData;
	fOutput_tree_optical = output_tree_opt;
	fOutput_tree_monitoring = output_tree_mon;

	if (fSave_data) {
		if (fCollect_optical) { addOptTreeBranches(); }
		if (fCollect_monitoring) { addMonTreeBranches(); }
	}
}

void DAQ_clb_handler::workOpticalData() {
	if (fCollect_optical && *fRunning == true) {
		fSocket_optical->async_receive(boost::asio::buffer(&fBuffer_optical[0], fBuffer_size),
								   boost::bind(&DAQ_clb_handler::handleOpticalData, this,
								   boost::asio::placeholders::error,
								   boost::asio::placeholders::bytes_transferred));
	}
}

void DAQ_clb_handler::workMonitoringData() {
	if (fCollect_monitoring && *fRunning == true) {
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

		// We have a successful optical packet, increment counter and print if required
		fCounter_optical ++;
		if (fCounter_optical % TERMINALPRINTRATE == 0) {
			std::cout << "DAQonite - Received: " << fCounter_optical << " optical packets" << std::endl;
		}

		if (fSave_data) {
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
											+ sizeof(CLBCommonHeader) + i
											* sizeof(hit_t)));

					fChannel_optical = (UChar_t)hit->channel;

					uint8_t time1 = hit->timestamp1;
					uint8_t time2 = hit->timestamp2;
					uint8_t time3 = hit->timestamp3;
					uint8_t time4 = hit->timestamp4;

					// Need to change the ordering of the bytes to get the correct hit time
					uint32_t orderedTime = (((uint32_t)time1) << 24) + (((uint32_t)time2) << 16) + (((uint32_t)time3) << 8) + ((uint32_t)time4);
					fTimestamp_ns_optical = (TimeStampNSTicks * 16) + (UInt_t)orderedTime;

					fTot_optical = (UChar_t)hit->ToT;

					if(*fRunning == true) { fOutput_tree_optical->Fill(); }
					
				}
			}
		}
		workOpticalData();
	}
}

void DAQ_clb_handler::handleMonitoringData(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		if (fBuffer_size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "DAQonite - Error: Invalid buffer size MONI: " << fBuffer_size << std::endl;
			workMonitoringData(); // DO I JUST WANT TO RETURN HERE?????
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

		// We have a successful monitoring packet, increment counter and print if required
		fCounter_monitoring ++;
		if (fCounter_monitoring % TERMINALPRINTRATE == 0) {
			std::cout << "DAQonite - Received: " << fCounter_monitoring << " monitoring packets" << std::endl;
		}

		// Get what we need from the header
		fPomId_monitoring = (UInt_t) header_monitoring.pomIdentifier();
		fTimestamp_s_monitoring = (UInt_t) header_monitoring.timeStamp().sec();

		// Get the monitoring hits data
		unsigned int hits[30];
		for (int i = 0; i < 30; ++i) {
			const uint32_t
					* const field =
							static_cast<const uint32_t* const >
									(static_cast<const void* const >(&fBuffer_monitoring[0] + sizeof(CLBCommonHeader) + i * 4));
			hits[i] = (unsigned int)htonl(*field);
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
											  hits, fTemperate_monitoring, fHumidity_monitoring);
			}

			if(*fRunning == true && fSave_data == true) { fOutput_tree_monitoring->Fill(); }
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

