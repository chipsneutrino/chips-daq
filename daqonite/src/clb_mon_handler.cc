/**
 * CLBMonHandler - Handler class for the CLB monitoring data stream
 */

#include "clb_mon_handler.h"

CLBMonHandler::CLBMonHandler(boost::asio::io_service* io_service, 
							 MonitoringGui *daqGui,
							 DataHandler *dataHandler,
							 bool *mode) :
					   		 fDaq_gui(daqGui), 
							 fData_handler(dataHandler), 
							 fMode(mode),
							 fBuffer_size(buffer_size_mon),
					   		 fSocket_monitoring(*io_service, udp::endpoint(udp::v4(), default_moni_port)) {

	// Setup the sockets
	udp::socket::receive_buffer_size option_clb(33554432);
	fSocket_monitoring.set_option(option_clb);
}

CLBMonHandler::~CLBMonHandler() {
	// Empty
}

void CLBMonHandler::workMonitoringData() {
	fSocket_monitoring.async_receive(boost::asio::buffer(&fBuffer_monitoring[0], fBuffer_size),
									 boost::bind(&CLBMonHandler::handleMonitoringData, this,
									 boost::asio::placeholders::error,
									 boost::asio::placeholders::bytes_transferred));
}

void CLBMonHandler::handleMonitoringData(boost::system::error_code const& error, std::size_t size) {
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
			  							  fData_handler->fTemperate_mon_clb, fData_handler->fHumidity_mon_clb);
		}

		if (*fMode == true) { fData_handler->fillMonCLBTree(); }

		workMonitoringData();
	} else {
		std::cout << "daqonite - Error: Monitoring async_receive!" << std::endl;
	}
}