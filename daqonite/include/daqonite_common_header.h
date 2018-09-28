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

// std and boost
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
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
#include <ctime>
#include <ostream>
#include <cassert>
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <sys/ioctl.h>
#include <signal.h>
#include <arpa/inet.h>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/property_tree/json_parser.hpp>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include <TRandom.h>
#include <TApplication.h>

// fh_library
#include "fh_library.h"

// DAQonite
//#include "bbb_api.h"
//#include "bbb_handler.h"
//#include "clb_data_structs.h"
//#include "clb_handler.h"
//#include "clb_header_structs.h"
//#include "daq_handler.h"
//#include "monitoring_plots.h"

#endif /* DAQ_HANDLER_H_ */
