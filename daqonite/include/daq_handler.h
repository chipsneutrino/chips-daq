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
#include "monitoring_plots.h"
#include "daqonite_gui.h"

namespace po = boost::program_options;
using boost::asio::ip::udp;

const static size_t buffer_size = 10000;
const static unsigned int default_opto_port = 56015;
const static unsigned int default_moni_port = 56017;

class DAQ_handler {
public:
	DAQ_handler(bool collect_clb_optical, bool collect_clb_monitoring,
			 	bool collect_bbb_optical, bool collect_bbb_monitoring,
			 	unsigned int optical_port, unsigned int monitoring_port,
			 	unsigned int bbb_port, bool save, std::string fileName,
			 	bool showGui);

	virtual ~DAQ_handler();

	void startRun(unsigned int runNum, unsigned int type);
	void pauseRun();
	void restartRun();
	void stopRun();
	void exit();

	void handle_signal(boost::asio::signal_set& set,
					   boost::system::error_code const& error, int signum);

	void handleGui();

	void SetCollectCLBOptical(bool val) {
		fCollect_CLB_optical_data = val;
	}
	bool GetCollectCLBOptical() {
		return fCollect_CLB_optical_data;
	}

	void SetCollectCLBMonitoring(bool val) {
		fCollect_CLB_monitoring_data = val;
	}
	bool GetCollectCLBMonitoring() {
		return fCollect_CLB_monitoring_data;
	}

	void SetCollectBBBOptical(bool val) {
		fCollect_BBB_optical_data = val;
	}
	bool GetCollectBBBOptical() {
		return fCollect_BBB_optical_data;
	}

	void SetCollectBBBMonitoring(bool val) {
		fCollect_BBB_monitoring_data = val;
	}
	bool GetCollectBBBMonitoring() {
		return fCollect_BBB_monitoring_data;
	}

	void SetCLBOpticalPort(unsigned int val) {
		fCLB_optical_port = val;
	}
	unsigned int GetCLBOpticalPort() {
		return fCLB_optical_port;
	}

	void SetCLBMonitoringPort(unsigned int val) {
		fCLB_monitoring_port = val;
	}
	unsigned int GetCLBMonitoringPort() {
		return fCLB_monitoring_port;
	}

	void SetBBBPort(unsigned int val) {
		fBBB_port = val;
	}
	unsigned int GetBBBPort() {
		return fBBB_port;
	}

	void SetFilename(std::string val) {
		fFilename = val;
	}
	std::string GetFilename() {
		return fFilename;
	}

private:
	// What do we want to collect?
	bool 				fCollect_CLB_optical_data;
	bool 				fCollect_CLB_monitoring_data;
	bool 				fCollect_BBB_optical_data;
	bool 				fCollect_BBB_monitoring_data;

	// The ports for the various things...
	unsigned int 		fCLB_optical_port;
	unsigned int 		fCLB_monitoring_port;
	unsigned int 		fBBB_port;

	// Output variables
	bool 				fSaveData;
	std::string 		fFilename;
	TFile* 				fOutput_file;
	TTree* 				fCLB_optical_tree;
	TTree* 				fCLB_monitoring_tree;
	TTree* 				fBBB_optical_tree;
	TTree* 				fBBB_monitoring_tree;

	// IO
	boost::asio::io_service fIO_service;
	CLB_handler* 		fCLB_handler;
	BBB_handler* 		fBBB_handler;

	// Combined things
	bool 				fShowGui;
	DAQoniteGUI* 		fDaqGui;
	std::size_t const 	fBuffer_size;
	bool 				fRunning;
};

#endif /* DAQ_HANDLER_H_ */
