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
	 					 unsigned int optical_port, unsigned int monitoring_port,
	 					 unsigned int bbb_port, bool save, std::string fileName,
	 					 bool showGui) :
						 fCollect_CLB_optical_data(collect_clb_optical),
						 fCollect_CLB_monitoring_data(collect_clb_monitoring),
						 fCollect_BBB_optical_data(collect_bbb_optical),
						 fCollect_BBB_monitoring_data(collect_bbb_monitoring),
						 fCLB_optical_port(optical_port),
						 fCLB_monitoring_port(monitoring_port),
						 fBBB_port(bbb_port), fSaveData(save), fFilename(fileName),
						 fShowGui(showGui), fBuffer_size(buffer_size) {

	fOutput_file = NULL;
	fCLB_optical_tree = NULL;
	fCLB_monitoring_tree = NULL;
	fBBB_optical_tree = NULL;
	fBBB_monitoring_tree = NULL;

	if (fShowGui) {
		fDaqGui = new DAQoniteGUI(gClient->GetRoot(),600,400);
	} else {
		fDaqGui = NULL;
	}

	// Set up the IO service
	fIO_service = new boost::asio::io_service();

	fSocket_clb_opt = new udp::socket(*fIO_service, udp::endpoint(udp::v4(), fCLB_optical_port));
	udp::socket::receive_buffer_size option_clb_opt(33554432);
	fSocket_clb_opt->set_option(option_clb_opt);

	fSocket_clb_mon = new udp::socket(*fIO_service, udp::endpoint(udp::v4(), fCLB_monitoring_port));
	udp::socket::receive_buffer_size option_clb_mon(33554432);
	fSocket_clb_mon->set_option(option_clb_mon);

	fSignalSet = new boost::asio::signal_set(*fIO_service, SIGINT, 34, 35);
	fSignalSet->async_wait(boost::bind(&DAQ_handler::handleSignal, this,
							   	   	   boost::asio::placeholders::error,
							   	   	   boost::asio::placeholders::signal_number));

	if (fCollect_CLB_optical_data || fCollect_CLB_monitoring_data) {
		fCLB_handler = new CLB_handler(fSocket_clb_opt, fCollect_CLB_optical_data,
									   fSocket_clb_mon, fCollect_CLB_monitoring_data,
									   fBuffer_size, fDaqGui);
	}

	// Setup the BBB handler
	if (fCollect_BBB_optical_data || fCollect_BBB_monitoring_data) {
		fBBB_handler = new BBB_handler();
		fBBB_handler->bbb_connect();
		fBBB_handler->get_bbb_status();
		fBBB_handler->bbb_disconnect();
	}

	fRunning = false;
}

DAQ_handler::~DAQ_handler() {
	if (fOutput_file!=NULL) {
		fOutput_file->Close();
		fOutput_file = NULL;
	}
	delete fIO_service;
	delete fSignalSet;
	delete fSocket_clb_opt;
	delete fSocket_clb_mon;
	delete fCLB_handler;
	delete fBBB_handler;
}

void DAQ_handler::startRun(unsigned int runNum, unsigned int type) {
	// Setup the output file
	TString fileName;
	if (fFilename == "") {
		fileName += "type";
		fileName += type;
		fileName += "_run";
		fileName += runNum;
		fileName += ".root";
	} else {
		fileName = fFilename;
	}
	if (fSaveData) {
		fOutput_file = new TFile(fileName, "RECREATE");
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

	fCLB_handler->SetSaveTrees(fSaveData, fCLB_optical_tree, fCLB_monitoring_tree);

	std::cout << "DAQonite - Start Mining... " << std::endl;
	if (fCollect_CLB_optical_data == true) {
		std::cout << "DAQonite - Will mine Opticalite on port: " << fCLB_optical_port << std::endl;
	}
	if (fCollect_CLB_monitoring_data == true) {
		std::cout << "DAQonite - Will mine Monitorite port: " << fCLB_monitoring_port << std::endl;
	}
	if (fSaveData) {
		std::cout << "DAQonite - Will arrange in the container: " << fileName << std::endl;
	}

	fCLB_handler->startData();
	fCLB_handler->work_optical_data();
	fCLB_handler->work_monitoring_data();
	handleGui();
	fRunning = true;
	fIO_service->run();
}

void DAQ_handler::pauseRun() {
	std::cout << "DAQonite - Pause Mining... " << std::endl;
	fRunning = false;
	fCLB_handler->stopData();
}

void DAQ_handler::restartRun() {
	std::cout << "DAQonite - Restart Mining... " << std::endl;
	fRunning = true;
	fCLB_handler->startData();
	fCLB_handler->work_optical_data();
	fCLB_handler->work_monitoring_data();
}

void DAQ_handler::stopRun() {
	std::cout << "DAQonite - Stop mining!" << std::endl;
	fRunning = false;
	fCLB_handler->stopData();
	if (fOutput_file != NULL) {
		if (fCLB_optical_tree != NULL) { fCLB_optical_tree->Write(); }
		if (fCLB_monitoring_tree != NULL) { fCLB_monitoring_tree->Write(); }
		fOutput_file->Close();
	}
	exit();
}

void DAQ_handler::exit() {
	std::cout << "DAQonite - Leave for the day!" << std::endl;
	fIO_service->stop();
	gApplication->Terminate(0);
}

void DAQ_handler::handleSignal(boost::system::error_code const& error, int signum) {
	if (!error) {
		if (signum == SIGINT) {
			stopRun();
			return;
		} else if (signum == 34) {
			pauseRun();
		} else if (signum == 35) {
			restartRun();
		}

		fSignalSet->async_wait(boost::bind(&DAQ_handler::handleSignal, this,
					   	   	   boost::asio::placeholders::error,
					   	   	   boost::asio::placeholders::signal_number));
	}
}

void DAQ_handler::handleGui() {
	// I could deal with the GUI timing in here!!!
	gSystem->ProcessEvents();
	fIO_service->post(boost::bind(&DAQ_handler::handleGui, this));
}
