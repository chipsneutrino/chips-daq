/*
 * clb_handler.cc
 * Handler for the CLB data
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#include "clb_handler.h"

CLB_handler::CLB_handler(boost::asio::ip::udp::socket* socket_opt, bool collect_opt,
						 boost::asio::ip::udp::socket* socket_mon, bool collect_mon,
						 std::size_t buffer_size, DAQoniteGUI *daqGui) :
						 fSocket_optical(socket_opt), fCollect_optical(collect_opt),
						 fSocket_monitoring(socket_mon), fCollect_monitoring(collect_mon),
						 fBuffer_size(buffer_size), fDaq_gui(daqGui) {

	// Construct not running
	fData_taking = false;

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
	fTemperate_monitoring 	= 0;
	fHumidity_monitoring 	= 0;

	// Packet Counters
	fCounter_optical 		= 0;
	fCounter_monitoring 		= 0;
}

CLB_handler::~CLB_handler() {
	// Empty
}

void CLB_handler::setSaveTrees(bool saveData, TTree * output_tree_opt, TTree * output_tree_mon) {
	fSave_data = saveData;
	fOutput_tree_optical = output_tree_opt;
	fOutput_tree_monitoring = output_tree_mon;

	if (fSave_data) {
		if (fCollect_optical) { addOptTreeBranches(); }
		if (fCollect_monitoring) { addMonTreeBranches(); }
	}
}

void CLB_handler::workOpticalData() {
	if (fCollect_optical && fData_taking) {
		fSocket_optical->async_receive(boost::asio::buffer(&fBuffer_optical[0], fBuffer_size),
								   boost::bind(&CLB_handler::handleOpticalData, this,
								   boost::asio::placeholders::error,
								   boost::asio::placeholders::bytes_transferred));
	}
}

void CLB_handler::workMonitoringData() {
	if (fCollect_monitoring && fData_taking) {
		fSocket_monitoring->async_receive(boost::asio::buffer(&fBuffer_monitoring[0], fBuffer_size),
								   boost::bind(&CLB_handler::handleMonitoringData, this,
								   boost::asio::placeholders::error,
								   boost::asio::placeholders::bytes_transferred));
	}
}

void CLB_handler::startData() {
	fData_taking = true;
}

void CLB_handler::stopData() {
	fData_taking = false;
}

void CLB_handler::addOptTreeBranches() {
	fOutput_tree_optical->Branch("PomId", &fPomId_optical, "fPomId_optical/i");
	fOutput_tree_optical->Branch("Channel", &fChannel_optical, "fChannel_optical/b");
	fOutput_tree_optical->Branch("TimeStamp_ns", &fTimestamp_ns_optical, "fTimestamp_ns_optical/i");
	fOutput_tree_optical->Branch("ToT", &fTot_optical, "fTot_optical/B");
}

void CLB_handler::addMonTreeBranches() {
	fOutput_tree_monitoring->Branch("PomId", &fPomId_monitoring, "fPomId_monitoring/i");
	fOutput_tree_monitoring->Branch("TimeStamp_s", &fTimestamp_s_monitoring, "fTimestamp_s_monitoring/i");
	fOutput_tree_monitoring->Branch("Pad", &fPad_monitoring, "fPad_monitoring/i");
	fOutput_tree_monitoring->Branch("Valid", &fValid_monitoring, "fValid_monitoring/i");
	fOutput_tree_monitoring->Branch("Temperate", &fTemperate_monitoring, "fTemperate_monitoring/i");
	fOutput_tree_monitoring->Branch("Humidity", &fHumidity_monitoring, "fHumidity_monitoring/i");
}

void CLB_handler::handleOpticalData(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		if (fBuffer_size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "Invalid buffer size OPTO: " << fBuffer_size << std::endl;
			workOpticalData();
			return;
		}

		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "Invalid packet size OPTO: " << size << std::endl;
			workOpticalData();
			return;
		}

		CLBCommonHeader const
				& header_optical =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fBuffer_optical[0]));

		// Check the type...
		std::pair<int, std::string> const& type = getType(header_optical);
		if (type.first != OPTO) { throw std::runtime_error("DAQonite: Error: Incorrect Type Not OPTO!"); }

		// We have a successful optical packet, increment counter and print if required
		fCounter_optical ++;
		if (fCounter_optical % TERMINALPRINTRATE == 0) {
			std::cout << "Received: " << fCounter_optical << " optical packets" << std::endl;
		}

		if (fSave_data) {
			fPomId_optical = (UInt_t) header_optical.pomIdentifier();
			fTimestamp_s_optical = (UInt_t) header_optical.timeStamp().sec();

			UInt_t TimeStampNSTicks = (UInt_t)header_optical.timeStamp().tics();
			fTimestamp_w_optical = TimeStampNSTicks*16;

		    if (((size - sizeof(CLBCommonHeader)) % sizeof(hit_t))!=0) {throw std::runtime_error("DAQonite: Error: Back Packet OPTO!");}
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

					fOutput_tree_optical->Fill();
				}
			}
		}
		workOpticalData();
	}
}

void CLB_handler::handleMonitoringData(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		if (fBuffer_size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "Invalid buffer size MONI: " << fBuffer_size << std::endl;
			workMonitoringData(); // DO I JUST WANT TO RETURN HERE?????
			return;
		}

		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "Invalid packet size MONI: " << size << std::endl;
			workMonitoringData();
			return;
		}

		CLBCommonHeader const
				& header_monitoring =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fBuffer_monitoring[0]));

		// Check the type...
		std::pair<int, std::string> const& type = getType(header_monitoring);
		if (type.first != MONI) { throw std::runtime_error("DAQonite: Error: Incorrect Type Not MONI!"); }

		// We have a successful monitoring packet, increment counter and print if required
		fCounter_monitoring ++;
		if (fCounter_monitoring % TERMINALPRINTRATE == 0) {
			std::cout << "Received: " << fCounter_monitoring << " monitoring packets" << std::endl;
		}

		fPomId_monitoring = (UInt_t) header_monitoring.pomIdentifier();
		fTimestamp_s_monitoring = (UInt_t) header_monitoring.timeStamp().sec();

		// Monitoring Plots
		if (fDaq_gui != NULL) {
			for (int i = 0; i < 30; ++i) { // NOT 31 HERE!!!
				const uint32_t
						* const field =
								static_cast<const uint32_t* const >
										(static_cast<const void* const >(&fBuffer_monitoring[0] + sizeof(CLBCommonHeader) + i * 4));

				unsigned int hits =	(unsigned int)htonl(*field);
				fDaq_gui->addHits((unsigned int)fPomId_monitoring, (unsigned int)i, hits);
			}
			fDaq_gui->addHeader(fPomId_monitoring, (UInt_t)header_monitoring.timeStamp().inMilliSeconds());
		}

		const ssize_t minimum_size = sizeof(CLBCommonHeader) + sizeof(int) * 31;

		if (fSave_data && ((ssize_t)size > minimum_size)) {
			const SCData
					* const scData =
							static_cast<const SCData* const > (static_cast<const void* const > (&fBuffer_monitoring[0]
									+ minimum_size));

			fPad_monitoring = (UInt_t)scData->pad;
			fValid_monitoring = (UInt_t)scData->valid;
			fTemperate_monitoring = (UInt_t)scData->temp;
			fHumidity_monitoring = (UInt_t)scData->humidity;

			fOutput_tree_monitoring->Fill();
		}

		workMonitoringData();
	}
}

std::pair<int, std::string> CLB_handler::getType(CLBCommonHeader const& header) {
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

void CLB_handler::printHeader(CLBCommonHeader const& header) {
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

void CLB_handler::printOpticalData(const char* const buffer, ssize_t buffer_size,
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

void CLB_handler::printMonitoringData(const char* const buffer, ssize_t buffer_size,
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

