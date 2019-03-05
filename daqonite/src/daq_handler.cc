/**
 * DAQHandler - Handler class for combining data streams
 */

#include "daq_handler.h"

DAQHandler::DAQHandler(bool collect_clb_data, bool collect_bbb_data,
					   bool gui, int numThreads, std::string configFile) :
					   fCollect_clb_data(collect_clb_data),
					   fCollect_bbb_data(collect_bbb_data),
					   fShow_gui(gui), fNum_threads(numThreads),
					   fData_handler(fCollect_clb_data, fCollect_bbb_data) {

	// 1) Set to monitoring mode
	fMode = false;

	// 2) Setup the io_service
	fIO_service = new boost::asio::io_service();

	// 3) Setup the signal listener
	fSignal_set = new boost::asio::signal_set(*fIO_service, SIGINT);
	workSignals();

	// 4) Setup the local socket (DAQommand) listener
	fLocal_socket = new udp::socket(*fIO_service, udp::endpoint(udp::v4(), 1096));
	udp::socket::receive_buffer_size option_local(33554432);
	fLocal_socket->set_option(option_local);
	workLocalSocket();

	// 5) Setup the monitoring GUI and timers (if required)
	if (fShow_gui) {
		fGui_event_timer = new boost::asio::deadline_timer(*fIO_service, boost::posix_time::millisec(GUIROOTRATE));
		fGui_update_timer = new boost::asio::deadline_timer(*fIO_service, boost::posix_time::millisec(GUIUPDATERATE));
		fDaq_gui = new MonitoringGui(GUIUPDATERATE, configFile);
		workGuiEvents();
		workGuiUpdate();
	} else { 
		fGui_event_timer = NULL; 
		fGui_update_timer = NULL; 
		fDaq_gui = NULL; 
	}

	// 6) Setup the CLB handler (if required)
	if (fCollect_clb_data) {
		fCLB_handler = new CLBHandler(fIO_service, fDaq_gui, &fData_handler, &fMode);
		fCLB_handler->workMonitoringData();
	} else { fCLB_handler = NULL; }

	// 7) Setup the BBB handler (if required)
	if (fCollect_bbb_data) {
		fBBB_handler = new BBBHandler();
	} else { fBBB_handler = NULL; }

	// 8) Setup the thread group and call io_service.run() in each
	std::cout << "daqonite - Starting IO service With " << fNum_threads << " threads" << std::endl;
	std::cout << "daqonite - Waiting for DAQommand command..." << std::endl;
	fThread_group = new boost::thread_group();
	for (int threadCount = 0; threadCount < fNum_threads; threadCount ++) {
		fThread_group->create_thread( boost::bind(&DAQHandler::ioServiceThread, this) );
	}

	// 9) Wait for all the threads to finish
	fThread_group->join_all();

	// 10) Terminate the ROOT gApplication (if required)
	if (fDaq_gui != NULL) {	
		std::cout << "daqonite - Terminating ROOT gApplication" << std::endl;
		gApplication->Terminate(0);	
	}
}

DAQHandler::~DAQHandler() {
	delete fIO_service;
	delete fSignal_set;
	delete fCLB_handler;
	delete fBBB_handler;
}

void DAQHandler::ioServiceThread() {
	fIO_service->run();
	std::cout << "daqonite - Thread Ending" << std::endl;
}

void DAQHandler::handleSignals(boost::system::error_code const& error, int signum) {
	if (!error) {
		if (signum == SIGINT) {
			if (fMode) {
				std::cout << "daqonite - Stopping current run first" << std::endl;

				// Set the mode to monitoring
				fMode = false;

				// Send stopRun() to the GUI so it can reset itself
				if (fDaq_gui != NULL) { fDaq_gui->stopRun(); }	

				// Stop the data_handler run
				fData_handler.stopRun();
			}
			std::cout << "\nDAQonite - Done for the day" << std::endl;
			fIO_service->stop();
			return;
		}

		// Incase we want to add other signals, you need to call the work method again
		workSignals();
	}
}

void DAQHandler::workSignals() {
	fSignal_set->async_wait(boost::bind(&DAQHandler::handleSignals, this,
					   	    boost::asio::placeholders::error,
					   	    boost::asio::placeholders::signal_number));
}

void DAQHandler::handleLocalSocket(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		if (strncmp(fBuffer_local, "start", 5) == 0) {
			// If we are currently running first stop the current run
			if (fMode == true) {
				std::cout << "daqonite - Stopping current run first" << std::endl;

				// Set the mode to monitoring
				fMode = false;

				// Send stopRun() to the GUI so it can reset itself
				if (fDaq_gui != NULL) { fDaq_gui->stopRun(); }	

				// Stop the data_handler run
				fData_handler.stopRun();
			}

			// Start a data_handler run
			fData_handler.startRun((int)fBuffer_local[5]-48);

			// Give run info to the GUI
			if (fDaq_gui != NULL) { 
				fDaq_gui->startRun(fData_handler.getRunType(), 
								   fData_handler.getRunNum(), 
								   fData_handler.getOutputName()); 
			}

			// Set the mode to data taking
			fMode = true;

			// Call the first work method to the optical data
			fCLB_handler->workOpticalData();
		} else if (strncmp(fBuffer_local, "stop", 4) == 0) {
			// Check we are actually running
			if (fMode == true) {
				// Set the mode to monitoring
				fMode = false;

				// Send stopRun() to the GUI so it can reset itself
				if (fDaq_gui != NULL) { fDaq_gui->stopRun(); }

				// Stop the data_handler run
				fData_handler.stopRun();			
			} else { 
				std::cout << "\nDAQonite - Already stopped mining" << std::endl;
			}
		} else if (strncmp(fBuffer_local, "exit", 4) == 0) {
			if (fMode == true) {
				std::cout << "daqonite - Stopping current run first" << std::endl;

				// Set the mode to monitoring
				fMode = false;

				// Send stopRun() to the GUI so it can reset itself
				if (fDaq_gui != NULL) { fDaq_gui->stopRun(); }	

				// Stop the data_handler run
				fData_handler.stopRun();
			}
			std::cout << "\nDAQonite - Done for the day" << std::endl;
			fIO_service->stop();
			return;
		} else {
			std::cout << "\nDAQonite - Error: Don't understand the command!\n" << std::endl;
		}

		workLocalSocket();
	}
}

void DAQHandler::workLocalSocket() {
	fLocal_socket->async_receive(boost::asio::buffer(&fBuffer_local[0], buffer_size),
								 boost::bind(&DAQHandler::handleLocalSocket, this,
								 boost::asio::placeholders::error,
								 boost::asio::placeholders::bytes_transferred));
}

void DAQHandler::handleGuiUpdate() {
	// This takes ~200 milliseconds so we call it as a seperate piece of work on the io_service
	fDaq_gui->update();
}

void DAQHandler::workGuiUpdate() {

	// Can use code bellow to get the current time, useful for debugging
	//boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1970,1,1)); 
	//boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	//boost::posix_time::time_duration diff = now - time_t_epoch;
	//std::cout << diff.total_milliseconds() << std::endl;
	
	fIO_service->post(boost::bind(&DAQHandler::handleGuiUpdate, this));

	fGui_update_timer->expires_from_now(boost::posix_time::millisec(GUIUPDATERATE));
	fGui_update_timer->async_wait(boost::bind(&DAQHandler::workGuiUpdate, this));
}

void DAQHandler::workGuiEvents() {
	gSystem->ProcessEvents();
	fGui_event_timer->expires_from_now(boost::posix_time::millisec(GUIROOTRATE));
	fGui_event_timer->async_wait(boost::bind(&DAQHandler::workGuiEvents, this));
}




