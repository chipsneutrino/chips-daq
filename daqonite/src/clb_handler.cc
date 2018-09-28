/*
 * clb_handler.cc
 * Handler for the CLB data
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#include "clb_handler.h"

CLB_handler::CLB_handler(boost::asio::ip::udp::socket& socket_opt, char* buffer_opt, bool collect_opt,
						 boost::asio::ip::udp::socket& socket_mon, char* buffer_mon, bool collect_mon,
						 bool saveData, std::size_t buffer_size, Monitoring_plots *monitoring_plots,
						 TTree * output_tree_opt, TTree * output_tree_mon) :
						 fSocket_optical(socket_opt), fBuffer_optical(buffer_opt), fCollect_optical(collect_opt),
						 fSocket_monitoring(socket_mon), fBuffer_monitoring(buffer_mon), fCollect_monitoring(collect_mon),
						 fBuffer_size(buffer_size), fSaveData(saveData), fMonitoringPlots(monitoring_plots),
						 fOutputTreeOptical(output_tree_opt), fOutputTreeMonitoring(output_tree_mon) {

	// Output Variables Optical
	fPomId_optical 			= 0;
	fTimeStamp_s_optical	= 0;
	fTimeStamp_w_optical 	= 0;
	fTimeStamp_ns_optical	= 0;

	// Output Variables Monitoring
	fPomId_monitoring 		= 0;
	fTimeStamp_s_monitoring = 0;
	fPad_monitoring 		= 0;
	fValid_monitoring 		= 0;
	fTemperate_monitoring 	= 0;
	fHumidity_monitoring 	= 0;

	// Packet Counters
	fCounterOptical 		= 0;
	fCounterMonitoring 		= 0;

	if (fSaveData) {
		if (fCollect_optical) { add_opt_tree_branches(); }
		if (fCollect_monitoring) { add_mon_tree_branches(); }
	}
}

CLB_handler::~CLB_handler() {
	// Empty
}

void CLB_handler::work_optical_data() {
	if (fCollect_optical) {
		fSocket_optical.async_receive(boost::asio::buffer(fBuffer_optical, fBuffer_size),
								   boost::bind(&CLB_handler::handle_optical_data, this,
								   boost::asio::placeholders::error,
								   boost::asio::placeholders::bytes_transferred));
	}
}

void CLB_handler::work_monitoring_data() {
	if (fCollect_monitoring) {
		fSocket_monitoring.async_receive(boost::asio::buffer(fBuffer_monitoring, fBuffer_size),
								   boost::bind(&CLB_handler::handle_monitoring_data, this,
								   boost::asio::placeholders::error,
								   boost::asio::placeholders::bytes_transferred));
	}
}

void CLB_handler::add_opt_tree_branches() {
	fOutputTreeOptical->Branch("PomId", &fPomId_optical, "fPomId_optical/i");
	fOutputTreeOptical->Branch("Channel", &fChannel_optical, "fChannel_optical/b");
	fOutputTreeOptical->Branch("TimeStamp_ns", &fTimeStamp_ns_optical, "fTimeStamp_ns_optical/i");
	fOutputTreeOptical->Branch("ToT", &fToT_optical, "fToT_optical/B");
}

void CLB_handler::add_mon_tree_branches() {
	fOutputTreeMonitoring->Branch("PomId", &fPomId_monitoring, "fPomId_monitoring/i");
	fOutputTreeMonitoring->Branch("TimeStamp_s", &fTimeStamp_s_monitoring, "fTimeStamp_s_monitoring/i");
	fOutputTreeMonitoring->Branch("Pad", &fPad_monitoring, "fPad_monitoring/i");
	fOutputTreeMonitoring->Branch("Valid", &fValid_monitoring, "fValid_monitoring/i");
	fOutputTreeMonitoring->Branch("Temperate", &fTemperate_monitoring, "fTemperate_monitoring/i");
	fOutputTreeMonitoring->Branch("Humidity", &fHumidity_monitoring, "fHumidity_monitoring/i");
}

void CLB_handler::handle_optical_data(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		if (fBuffer_size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "Invalid buffer size OPTO: " << fBuffer_size << std::endl;
			work_optical_data();
			return;
		}

		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "Invalid packet size OPTO: " << size << std::endl;
			work_optical_data();
			return;
		}

		CLBCommonHeader const
				& header_optical =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (fBuffer_optical));

		// Check the type...
		std::pair<int, std::string> const& type = getType(header_optical);
		if (type.first != OPTO) { throw std::runtime_error("DAQonite: Error: Incorrect Type Not OPTO!"); }

		// We have a successful optical packet, increment counter and print if required
		fCounterOptical ++;
		if (fCounterOptical % TERMINALPRINTRATE == 0) {
			std::cout << "Received: " << fCounterOptical << " optical packets" << std::endl;
		}

		if (fSaveData) {
			fPomId_optical = (UInt_t) header_optical.pomIdentifier();
			fTimeStamp_s_optical = (UInt_t) header_optical.timeStamp().sec();

			UInt_t TimeStampNSTicks = (UInt_t)header_optical.timeStamp().tics();
			fTimeStamp_w_optical = TimeStampNSTicks*16;

		    if (((size - sizeof(CLBCommonHeader)) % sizeof(hit_t))!=0) {throw std::runtime_error("DAQonite: Error: Back Packet OPTO!");}
			const unsigned int nhits = (size - sizeof(CLBCommonHeader)) / sizeof(hit_t);

			if (nhits) {
				for (int i = 0; i < (int)nhits; ++i) {
					const hit_t
							* const hit =
									static_cast<const hit_t* const > (static_cast<const void* const > (fBuffer_optical
											+ sizeof(CLBCommonHeader) + i
											* sizeof(hit_t)));

					fChannel_optical = (UChar_t)hit->channel;

					uint8_t time1 = hit->timestamp1;
					uint8_t time2 = hit->timestamp2;
					uint8_t time3 = hit->timestamp3;
					uint8_t time4 = hit->timestamp4;

					// Need to change the ordering of the bytes to get the correct hit time
					uint32_t orderedTime = (((uint32_t)time1) << 24) + (((uint32_t)time2) << 16) + (((uint32_t)time3) << 8) + ((uint32_t)time4);
					fTimeStamp_ns_optical = (TimeStampNSTicks * 16) + (UInt_t)orderedTime;

					fToT_optical = (UChar_t)hit->ToT;

					fOutputTreeOptical->Fill();
				}
			}
		}
		work_optical_data();
	}
}

void CLB_handler::handle_monitoring_data(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		if (fBuffer_size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "Invalid buffer size MONI: " << fBuffer_size << std::endl;
			work_monitoring_data(); // DO I JUST WANT TO RETURN HERE?????
			return;
		}

		if (size - sizeof(CLBCommonHeader) < 0) {
			std::cout << "Invalid packet size MONI: " << size << std::endl;
			work_monitoring_data();
			return;
		}

		CLBCommonHeader const
				& header_monitoring =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (fBuffer_monitoring));

		// Check the type...
		std::pair<int, std::string> const& type = getType(header_monitoring);
		if (type.first != MONI) { throw std::runtime_error("DAQonite: Error: Incorrect Type Not MONI!"); }

		// We have a successful monitoring packet, increment counter and print if required
		fCounterMonitoring ++;
		if (fCounterMonitoring % TERMINALPRINTRATE == 0) {
			std::cout << "Received: " << fCounterMonitoring << " monitoring packets" << std::endl;
		}

		fPomId_monitoring = (UInt_t) header_monitoring.pomIdentifier();
		fTimeStamp_s_monitoring = (UInt_t) header_monitoring.timeStamp().sec();

		// Monitoring Plots
		if (fMonitoringPlots != NULL) {
			for (int i = 0; i < 30; ++i) { // NOT 31 HERE!!!
				const uint32_t
						* const field =
								static_cast<const uint32_t* const >
										(static_cast<const void* const >(fBuffer_monitoring + sizeof(CLBCommonHeader) + i * 4));

				unsigned int hits =	(unsigned int)htonl(*field);
				fMonitoringPlots->addHits((unsigned int)fPomId_monitoring, (unsigned int)i, hits);
			}
			fMonitoringPlots->addHeader(fPomId_monitoring, (UInt_t)header_monitoring.timeStamp().inMilliSeconds());
		}

		const ssize_t minimum_size = sizeof(CLBCommonHeader) + sizeof(int) * 31;

		if (fSaveData && ((ssize_t)size > minimum_size)) {
			const SCData
					* const scData =
							static_cast<const SCData* const > (static_cast<const void* const > (fBuffer_monitoring
									+ minimum_size));

			fPad_monitoring = (UInt_t)scData->pad;
			fValid_monitoring = (UInt_t)scData->valid;
			fTemperate_monitoring = (UInt_t)scData->temp;
			fHumidity_monitoring = (UInt_t)scData->humidity;

			fOutputTreeMonitoring->Fill();
		}

		work_monitoring_data();
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

void CLB_handler::print_header(CLBCommonHeader const& header) {
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

void CLB_handler::print_optical_data(const char* const buffer, ssize_t buffer_size,
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

void CLB_handler::print_monitoring_data(const char* const buffer, ssize_t buffer_size,
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

