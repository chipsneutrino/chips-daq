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

	fRunning = false;
}

DAQ_handler::~DAQ_handler() {
	if (fOutput_file!=NULL) {
		fOutput_file->Close();
		fOutput_file = NULL;
	}
}

void DAQ_handler::StartRun() {

	// First setup the output file
	if (fSaveData) {
		fOutput_file = new TFile(fFilename.c_str(), "RECREATE");
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

	// Setup the CLB optical data socket
	udp::socket socket_clb_opt(fIO_service, udp::endpoint(udp::v4(), fCLB_optical_port));
	udp::socket::receive_buffer_size option_clb_opt(33554432);
	socket_clb_opt.set_option(option_clb_opt);
	char buffer_clb_opt[fBuffer_size] __attribute__((aligned(8)));

	// Setup the monitoring data socket
	udp::socket socket_clb_mon(fIO_service, udp::endpoint(udp::v4(), fCLB_monitoring_port));
	udp::socket::receive_buffer_size option_clb_mon(33554432);
	socket_clb_mon.set_option(option_clb_mon);
	char buffer_clb_mon[fBuffer_size] __attribute__((aligned(8)));

	// Setup the signals_handler
	boost::asio::signal_set signals_handler(fIO_service, SIGWINCH, SIGINT);

	//signals_handler.async_wait(handle_signal);

	signals_handler.async_wait(boost::bind(&DAQ_handler::handle_signal, this, boost::ref(signals_handler),
							   boost::asio::placeholders::error,
							   boost::asio::placeholders::signal_number));

	// Setup the CLB handler
	if (fCollect_CLB_optical_data || fCollect_CLB_monitoring_data) {
		fCLB_handler = new CLB_handler(socket_clb_opt, buffer_clb_opt, fCollect_CLB_optical_data,
									   socket_clb_mon, buffer_clb_mon, fCollect_CLB_monitoring_data,
									   fSaveData, fBuffer_size, fDaqGui,
									   fCLB_optical_tree, fCLB_monitoring_tree);
	}

	// Setup the BBB handler
	if (fCollect_BBB_optical_data || fCollect_BBB_monitoring_data) {
		fBBB_handler = new BBB_handler();
		fBBB_handler->bbb_connect();
		fBBB_handler->get_bbb_status();
		fBBB_handler->bbb_disconnect();
	}

	if (fCollect_CLB_optical_data == true) {
		std::cout << "DAQonite - Mining on port: " << fCLB_optical_port << ", for Optical data ... " << std::endl;
	}
	if (fCollect_CLB_monitoring_data == true) {
		std::cout << "DAQonite - Mining on port: " << fCLB_monitoring_port << ", for Monitoring data ... " << std::endl;
	}
	if (fSaveData) {
		std::cout << "DAQonite - Putting into container: " << fFilename << std::endl;
	}

	std::cout << "DAQonite - Start Mining... " << std::endl;
	fRunning = true;
	fCLB_handler->work_optical_data();
	fCLB_handler->work_monitoring_data();
	handleGui();
	fIO_service.run();
}

void DAQ_handler::StopRun() {
	std::cout << "DAQonite - Stop mining!" << std::endl;
	fRunning = false;
	fIO_service.stop();
	if (fOutput_file != NULL) {
		if (fCLB_optical_tree != NULL) { fCLB_optical_tree->Write(); }
		if (fCLB_monitoring_tree != NULL) { fCLB_monitoring_tree->Write(); }
		fOutput_file->Close();
	}
	gApplication->Terminate(0);
}

void DAQ_handler::handle_signal(boost::asio::signal_set& set,
				   	   	   	    boost::system::error_code const& error, int signum) {
	if (!error) {
		if (signum == SIGINT) {
			StopRun();
			return;
		}
		set.async_wait(boost::bind(&DAQ_handler::handle_signal, this, boost::ref(set),
					   boost::asio::placeholders::error,
					   boost::asio::placeholders::signal_number));
	}
}

void DAQ_handler::handleGui() {
	gSystem->ProcessEvents();
	fIO_service.post(boost::bind(&DAQ_handler::handleGui, this));  
}