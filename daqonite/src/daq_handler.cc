/*
 * daq_handler.cc
 * Handler for the combining of both streams of data
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#include "daq_handler.h"

DAQ_handler::DAQ_handler(bool collect_clb_optical, bool collect_clb_monitoring,
			 			 bool collect_bbb_optical, bool collect_bbb_monitoring,
						 bool save, bool gui, bool localControl) :
						 fCollect_CLB_optical_data(collect_clb_optical),
						 fCollect_CLB_monitoring_data(collect_clb_monitoring),
						 fCollect_BBB_optical_data(collect_bbb_optical),
						 fCollect_BBB_monitoring_data(collect_bbb_monitoring),
						 fSave_data(save), fShow_gui(gui), 
						 fLocal_control(localControl), fBuffer_size(buffer_size) {

	// NULL all the ROOT output variables, can then check this later
	fOutput_file = NULL;
	fCLB_optical_tree = NULL;
	fCLB_monitoring_tree = NULL;
	fBBB_optical_tree = NULL;
	fBBB_monitoring_tree = NULL;

	// Set up the IO service
	fIO_service = new boost::asio::io_service();

	// Add a signal listener to the IO service. Just listens for SIGINT (ctrl-c)
	fSignal_set = new boost::asio::signal_set(*fIO_service, SIGINT);
	workSignals();

	// Add the local control socket to the IO service
	// Need to have client connection before execution will continue
	if (fLocal_control) {
		fLocal_socket = new udp::socket(*fIO_service, udp::endpoint(udp::v4(), 1096));
		udp::socket::receive_buffer_size option_local(33554432);
		fLocal_socket->set_option(option_local);

		workLocalSocket();
	}

	// Set up the monitoring ROOT based GUI
	if (fShow_gui) {
		fDaq_gui = new DAQoniteGUI(gClient->GetRoot(),600,400);
		workGui();
	} else {
		fDaq_gui = NULL;
	}

	// TODO: Move this to the CLB Handler if possible
	// Add the CLB Optical socket to the IO service
	fSocket_clb_opt = new udp::socket(*fIO_service, udp::endpoint(udp::v4(), default_opto_port));
	udp::socket::receive_buffer_size option_clb_opt(33554432);
	fSocket_clb_opt->set_option(option_clb_opt);

	// Add the CLB monitoring port to the IO service
	fSocket_clb_mon = new udp::socket(*fIO_service, udp::endpoint(udp::v4(), default_moni_port));
	udp::socket::receive_buffer_size option_clb_mon(33554432);
	fSocket_clb_mon->set_option(option_clb_mon);

	// Create the CLB handler, deals with all CLB data collection
	if (fCollect_CLB_optical_data || fCollect_CLB_monitoring_data) {
		fCLB_handler = new CLB_handler(fSocket_clb_opt, fCollect_CLB_optical_data,
									   fSocket_clb_mon, fCollect_CLB_monitoring_data,
									   fBuffer_size, fDaq_gui);
	}

	// Create the BBB handler, deals with all BBB data collection
	if (fCollect_BBB_optical_data || fCollect_BBB_monitoring_data) {
		fBBB_handler = new BBB_handler();
		fBBB_handler->bbb_connect();
		fBBB_handler->get_bbb_status();
		fBBB_handler->bbb_disconnect();
	}

	// Start not running
	fRunning = false;

	// Start the IO service 
	fIO_service->run();
}

DAQ_handler::~DAQ_handler() {
	if (fOutput_file!=NULL) {
		fOutput_file->Close();
		fOutput_file = NULL;
	}
	delete fIO_service;
	delete fSignal_set;
	delete fSocket_clb_opt;
	delete fSocket_clb_mon;
	delete fCLB_handler;
	delete fBBB_handler;
}

void DAQ_handler::startRun(unsigned int runNum, unsigned int type) {
	// Setup the output file
	fFilename = "";
	fFilename += "type";
	fFilename += type;
	fFilename += "_run";
	fFilename += runNum;
	fFilename += ".root";
	if (fSave_data) {
		fOutput_file = new TFile(fFilename, "RECREATE");
		if (!fOutput_file) { throw std::runtime_error("Error: Opening Output file!"); }
		if (fCollect_CLB_optical_data) {
			fCLB_optical_tree = new TTree("CLBOpt_tree", "CLBOpt_tree");
			if (!fCLB_optical_tree) { throw std::runtime_error("Error: fCLB_optical_tree!"); }
		}
		if (fCollect_CLB_monitoring_data) {
			fCLB_monitoring_tree = new TTree("CLBMon_tree", "CLBMon_tree");
			if (!fCLB_monitoring_tree) { throw std::runtime_error("Error: fCLB_monitoring_tree!"); }
		}
		if (fCollect_BBB_optical_data) {
			fBBB_optical_tree = new TTree("BBBOpt_tree", "BBBOpt_tree");
			if (!fBBB_optical_tree) { throw std::runtime_error("Error: fBBB_optical_tree!"); }
		}
		if (fCollect_BBB_monitoring_data) {
			fBBB_monitoring_tree = new TTree("BBBMon_tree", "BBBMon_tree");
			if (!fBBB_monitoring_tree) { throw std::runtime_error("Error: fBBB_monitoring_tree!"); }
		}
	}

	fCLB_handler->setSaveTrees(fSave_data, fCLB_optical_tree, fCLB_monitoring_tree);

	std::cout << "\nDAQonite - Start Mining (Type: " << type << ", Run: " << runNum << ")" << std::endl;
	if (fCollect_CLB_optical_data == true) {
		std::cout << "DAQonite - Will mine Opticalite on port: " << default_opto_port << std::endl;
	}
	if (fCollect_CLB_monitoring_data == true) {
		std::cout << "DAQonite - Will mine Monitorite port: " << default_moni_port << std::endl;
	}
	if (fSave_data) {
		std::cout << "DAQonite - Will arrange in the container: " << fFilename << std::endl << std::endl;
	}

	fCLB_handler->startData();
	fCLB_handler->workOpticalData();
	fCLB_handler->workMonitoringData();
	fRunning = true;
}

void DAQ_handler::newRun(unsigned int runNum, unsigned int type) {
	// First stop the current run and save the .root output file
	stopRun();

	// Start a new run
	startRun(2,2);
}

void DAQ_handler::stopRun() {
	std::cout << "\nDAQonite - Stop mining!" << std::endl;
	fRunning = false;
	fCLB_handler->stopData();
	if (fOutput_file != NULL) {
		std::cout << "DAQonite - Writing and closing file" << std::endl;
		if (fCLB_optical_tree != NULL) { fCLB_optical_tree->Write(); }
		if (fCLB_monitoring_tree != NULL) { fCLB_monitoring_tree->Write(); }
		fOutput_file->Close();
	}
}

void DAQ_handler::exit() {
	std::cout << "DAQonite - Done for the day!" << std::endl;
	fIO_service->stop();
	gApplication->Terminate(0);
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
		std::cout << "Received local message of length -> " << size << std::endl;
		if (strncmp(fBuffer_local, "start", 5) == 0) {
			startRun(1,2);
		} else if (strncmp(fBuffer_local, "stop", 4) == 0) {
			stopRun();
		} else if (strncmp(fBuffer_local, "new", 3) == 0) {
			newRun(1,2);
		} else if (strncmp(fBuffer_local, "exit", 4) == 0) {
			exit();
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
	gSystem->ProcessEvents();
	fIO_service->post(boost::bind(&DAQ_handler::workGui, this));
}
