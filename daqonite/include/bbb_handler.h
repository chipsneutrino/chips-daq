/**
 * BBB_handler - Handler class for the BBB data stream
 * 
 * This class deals with the specifics of the BBB data stream, using the
 * fh_library API, to communicate with the Madison beaglebones.
 * 
 * NOTE: THIS IS A TEST IMPLEMENTATION!!!
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
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

		/// Create a BBB_handler
		BBB_handler();

		/// Destroy a CLB_handler
		virtual ~BBB_handler();

		/**
		 * Connects to the test server
		 * Opens a TCP socket to communicate with the 'beaglebone' server
		 * Using the COBS encoded frame protocal version 2.
		 */
		void bbb_connect();

		/**
		 * Gets the server status
		 * Sends the MS_STATUS binary message to the beaglebone and gets 
		 * and prints the response to stdout.
		 */
		void get_bbb_status();

		/**
		 * Disconnect from the server
		 * Sends the MS_CLOSE binary message to the beaglebone and checks 
		 * for response before connection is closed.
		 */
		void bbb_disconnect();

	private:

		char* 			server_ip;	///< Beaglebone server IP address
		uint16_t 		port;		///< TCP socket port

		fh_connector_t*	conn;		///< fh_library connector to setup connection
		fh_transport_t*	transport;	///< fh_library transport for message IO
		fh_message_t*	msg;		///< fh_library message implementation
};

#endif
