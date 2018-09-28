/*
 * bbb_handler.h
 * Handler for the beaglebone data
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#ifndef BBB_HANDLER_H_
#define BBB_HANDLER_H_

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

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"

#include "fh_library.h"
#include "bbb_api.h"

class BBB_handler {
	public:
		BBB_handler();
		virtual ~BBB_handler();

		void bbb_connect();
		void get_bbb_status();
		void bbb_disconnect();

	private:
		char* 			server_ip;
		uint16_t 		port;

		fh_connector_t*	conn;
		fh_transport_t*	transport;
		fh_message_t*	msg;
};

#endif /* BBB_HANDLER_H_ */
