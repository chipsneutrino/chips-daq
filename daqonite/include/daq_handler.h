/*
 * daq_handler.h
 * Handler that deals with combining data streams
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#ifndef DAQ_HANDLER_H_
#define DAQ_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include "assert.h"

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <sys/ioctl.h>
#include <signal.h>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include <TApplication.h>

#include "clb_handler.h"
#include "bbb_handler.h"
#include "daqonite_gui.h"

namespace po = boost::program_options;
using boost::asio::ip::udp;

#define NUMRUNTYPES 4

//const static size_t buffer_size = 10000;
const static unsigned int default_opto_port = 56015;
const static unsigned int default_moni_port = 56017;

class DAQ_handler {
public:
	DAQ_handler(bool collect_clb_optical, bool collect_clb_monitoring,
			 	bool collect_bbb_optical, bool collect_bbb_monitoring,
				bool gui, bool save, unsigned int runType);

	virtual ~DAQ_handler();

	void startRun();
	void newRun();
	void stopRun();
	void exit();

	int getRunAndUpdate();

	void handleSignals(boost::system::error_code const& error, int signum);
	void handleLocalSocket(boost::system::error_code const& error, std::size_t size);

	void workSignals();
	void workLocalSocket();
	void workGui();

private:
	// Settings
	bool 						fCollect_CLB_optical_data;
	bool 						fCollect_CLB_monitoring_data;
	bool 						fCollect_BBB_optical_data;
	bool 						fCollect_BBB_monitoring_data;
	bool 						fShow_gui;
	bool 						fSave_data;

	// Output variables
	TString 					fFilename;
	TFile* 						fOutput_file;
	TTree* 						fCLB_optical_tree;
	TTree* 						fCLB_monitoring_tree;
	TTree* 						fBBB_optical_tree;
	TTree* 						fBBB_monitoring_tree;

	// IO
	boost::asio::io_service* 	fIO_service;
	boost::asio::signal_set*	fSignal_set;
	udp::socket*				fLocal_socket;
	char 						fBuffer_local[buffer_size] __attribute__((aligned(8)));
	udp::socket*				fSocket_clb_opt;
	udp::socket*				fSocket_clb_mon;
	CLB_handler* 				fCLB_handler;
	BBB_handler* 				fBBB_handler;

	// Combined things
	DAQoniteGUI* 				fDaq_gui;
	unsigned int 				fRun_type;
	std::size_t const 			fBuffer_size;
	bool						fRunning;
};

#endif /* DAQ_HANDLER_H_ */
