/**
 * DAQ_handler - Handler class for combining data streams
 */

#include "DAQ_handler.h"

DAQ_handler::DAQ_handler(bool collect_clb_optical, bool collect_clb_monitoring,
			 			 bool collect_bbb_optical, bool collect_bbb_monitoring,
						 bool gui, bool save, int numThreads) :
						 fCollect_CLB_optical_data(collect_clb_optical),
						 fCollect_CLB_monitoring_data(collect_clb_monitoring),
						 fCollect_BBB_optical_data(collect_bbb_optical),
						 fCollect_BBB_monitoring_data(collect_bbb_monitoring),
						 fShow_gui(gui), fSave_data(save), fNum_threads(numThreads) {

	// How we want this to run...
	// 1) Set to monitoring mode and initialise run variables
	// 2) Setup the io_service
	// 3) Setup the signal listener
	// 4) Setup the local socket (DAQommand) listener
	// 5) Setup the monitoring GUI (if required)
	// 6) Setup the CLB handler (if required)
	// 7) Setup the BBB handler (if required)
	// 8) Setup the thread group and call io_service.run() in each
	// 9) Wait for all the threads to finish
	// 10) Terminate the ROOT gApplication (if required)

	// 1) Set to monitoring mode and initialise run variables
	fMode = false;

	fRun_type = -1;
	fOutput_file = NULL;
	fCLB_optical_tree = NULL;
	fCLB_monitoring_tree = NULL;
	fBBB_optical_tree = NULL;
	fBBB_monitoring_tree = NULL;

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

	// 5) Setup the monitoring GUI (if required)
	if (fShow_gui) {
		fDaq_gui = new Monitoring_gui();
		workGui();
	} else { fDaq_gui = NULL; }

	// 6) Setup the CLB handler (if required)
	if (fCollect_CLB_optical_data || fCollect_CLB_monitoring_data) {
		fCLB_handler = new DAQ_clb_handler(fIO_service, 
										   fCollect_CLB_optical_data, fCollect_CLB_monitoring_data,
									   	   fDaq_gui, &fMode);
	}

	// 7) Setup the BBB handler (if required)
	if (fCollect_BBB_optical_data || fCollect_BBB_monitoring_data) {
		fBBB_handler = new DAQ_bbb_handler();
	}

	// 8) Setup the thread group and call io_service.run() in each
	std::cout << "DAQonite - Starting IO service With " << fNum_threads << " threads" << std::endl;
	std::cout << "DAQonite - Waiting for DAQommand command..." << std::endl;
	fThread_group = new boost::thread_group();
	for (int threadCount = 0; threadCount < fNum_threads; threadCount ++) {
		fThread_group->create_thread( boost::bind(&DAQ_handler::ioServiceThread, this) );
	}

	// 9) Wait for all the threads to finish
	fThread_group->join_all();

	// 10) Terminate the ROOT gApplication (if required)
	if (fDaq_gui != NULL) {	gApplication->Terminate(0);	}
}

DAQ_handler::~DAQ_handler() {
	if (fOutput_file!=NULL) {
		fOutput_file->Close();
		fOutput_file = NULL;
	}
	delete fIO_service;
	delete fSignal_set;
	delete fCLB_handler;
	delete fBBB_handler;
}

void DAQ_handler::startRun() {
	// If we are currently running, stop the current run before starting a new one
	if (fMode == true) { stopRun(); }

	// Setup the output file
	if (fSave_data) {
		int runNum = getRunAndUpdate();
		fFilename = "../data/";
		fFilename += "type";
		fFilename += fRun_type;
		fFilename += "_run";
		fFilename += runNum;
		fFilename += ".root";

		fOutput_file = new TFile(fFilename, "RECREATE");
		if (!fOutput_file) { throw std::runtime_error("DAQonite - Error: Opening output file!"); }
		if (fCollect_CLB_optical_data) {
			fCLB_optical_tree = new TTree("CLBOpt_tree", "CLBOpt_tree");
			if (!fCLB_optical_tree) { throw std::runtime_error("DAQonite - Error: fCLB_optical_tree!"); }
		}
		if (fCollect_CLB_monitoring_data) {
			fCLB_monitoring_tree = new TTree("CLBMon_tree", "CLBMon_tree");
			if (!fCLB_monitoring_tree) { throw std::runtime_error("DAQonite - Error: fCLB_monitoring_tree!"); }
		}
		if (fCollect_BBB_optical_data) {
			fBBB_optical_tree = new TTree("BBBOpt_tree", "BBBOpt_tree");
			if (!fBBB_optical_tree) { throw std::runtime_error("DAQonite - Error: fBBB_optical_tree!"); }
		}
		if (fCollect_BBB_monitoring_data) {
			fBBB_monitoring_tree = new TTree("BBBMon_tree", "BBBMon_tree");
			if (!fBBB_monitoring_tree) { throw std::runtime_error("DAQonite - Error: fBBB_monitoring_tree!"); }
		}
		fCLB_handler->setSaveTrees(fSave_data, fCLB_optical_tree, fCLB_monitoring_tree);
		if (fDaq_gui != NULL) { fDaq_gui->startRun(fRun_type, runNum, fFilename); }
	} else {
		if (fDaq_gui != NULL) { fDaq_gui->startRun(fRun_type, 0, ""); }
	}

	std::cout << "\nDAQonite - Start mining on ( "<< default_opto_port << ", " << default_moni_port << " )..." << std::endl;
	if (fSave_data) {
		std::cout << "DAQonite - Filling container: " << fFilename << std::endl;
	}

	fMode = true;
	fCLB_handler->workOpticalData();
	fCLB_handler->workMonitoringData();
}

void DAQ_handler::stopRun() {
	if (fMode == true) {
		std::cout << "\nDAQonite - Stop mining" << std::endl;
		if (fDaq_gui != NULL) { fDaq_gui->stopRun(); }
		fMode = false;
		if (fOutput_file != NULL) {
			std::cout << "DAQonite - Closing the container: " << fFilename << std::endl;
			if (fCLB_optical_tree != NULL) { fCLB_optical_tree->Write(); }
			if (fCLB_monitoring_tree != NULL) { fCLB_monitoring_tree->Write(); }
			fOutput_file->Close();
		}		
	} else { std::cout << "\nDAQonite - Already stopped mining" << std::endl;}
}

void DAQ_handler::exit() {
	if (fMode == true) { stopRun(); }
	std::cout << "\nDAQonite - Done for the day" << std::endl;
	fIO_service->stop();
}

int DAQ_handler::getRunAndUpdate() {
	// 4 fRun_types -> 1) Data_normal, 2) Calibration, 3) Test_normal, 4) test_daq
	if (fRun_type < 0 || fRun_type >= NUMRUNTYPES) {
		throw std::runtime_error("DAQonite - Error: Incorrect run type number!");
	}

	int returnNum = 1;
	int runNums[NUMRUNTYPES];
	std::ifstream runNumFile("../data/runNumbers.dat");	
	if(runNumFile.fail()) {
		runNumFile.close();	
		// The file does not yet exist so lets create it
		std::ofstream newFile("../data/runNumbers.dat");
  		if (newFile.is_open()) {
			for (int i=0; i<NUMRUNTYPES; i++) {	
				if (fRun_type == (unsigned int)i) { 
					newFile << 2 << "\n"; 
				} else { newFile << 1 << "\n"; }
			}
			newFile.close();
		} else { throw std::runtime_error("DAQonite - Error: Unable to create ../data/runNumbers.dat!"); }
	} else {
		// The file exists so read from it
		for (int i=0; i<NUMRUNTYPES; i++) { 
			runNumFile >> runNums[i]; 
			if (runNums[i] < 1) { runNums[i] = 1; }
			if (fRun_type == (unsigned int)i) { returnNum = runNums[i]; }
		}
		runNumFile.close();	

		// The file does not yet exist so lets create it
		std::ofstream updateFile("../data/runNumbers.dat");
  		if (updateFile.is_open()) {
			for (int i=0; i<NUMRUNTYPES; i++) {	
				if (fRun_type == (unsigned int)i) { 
					updateFile << runNums[i] + 1 << "\n"; 
				} else { updateFile << 1 << "\n"; }
			}
			updateFile.close();
		} else { throw std::runtime_error("DAQonite - Error: Unable to update runNumbers.dat!"); }
	}

	return returnNum;
}

void DAQ_handler::ioServiceThread() {
	fIO_service->run();
}

void DAQ_handler::handleSignals(boost::system::error_code const& error, int signum) {
	if (!error) {
		if (signum == SIGINT) {
			stopRun();
			exit();
			return;
		}

		// Incase we want to add other signals, you need to call the work method again
		workSignals();
	}
}

void DAQ_handler::handleLocalSocket(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
		if (strncmp(fBuffer_local, "start", 5) == 0) {
			fRun_type = (int)fBuffer_local[5]-48;
			startRun();
		} else if (strncmp(fBuffer_local, "stop", 4) == 0) {
			stopRun();
		} else if (strncmp(fBuffer_local, "exit", 4) == 0) {
			exit();
			return;
		} else {
			std::cout << "\nDAQonite - Error: Don't understand the command!\n" << std::endl;
		}

		workLocalSocket();
	}
}

void DAQ_handler::workSignals() {
	fSignal_set->async_wait(boost::bind(&DAQ_handler::handleSignals, this,
					   	    boost::asio::placeholders::error,
					   	    boost::asio::placeholders::signal_number));
}

void DAQ_handler::workLocalSocket() {
	fLocal_socket->async_receive(boost::asio::buffer(&fBuffer_local[0], buffer_size),
								 boost::bind(&DAQ_handler::handleLocalSocket, this,
								 boost::asio::placeholders::error,
								 boost::asio::placeholders::bytes_transferred));
}

void DAQ_handler::workGui() {
	// TODO: Have this as an ASYNC_WAIT
	usleep(50);
	gSystem->ProcessEvents();
	fIO_service->post(boost::bind(&DAQ_handler::workGui, this));
}
