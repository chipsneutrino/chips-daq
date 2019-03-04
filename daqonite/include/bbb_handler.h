/**
 * BBBHandler - Handler class for the BBB data stream
 * 
 * This class deals with the specifics of the BBB data stream, using the
 * fh_library API, to communicate with the Madison beaglebones.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef BBB_HANDLER_H_
#define BBB_HANDLER_H_

#include <iostream>
#include "assert.h"

#include "fh_library.h"
#include "bbb_api.h"

class BBBHandler {
	public:
		/// Create a BBBHandler
		BBBHandler();

		/// Destroy a BBBHandler
		~BBBHandler();

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
		char* 				fServer_ip;		///< Beaglebone server IP address
		uint16_t 			fPort;			///< TCP socket port

		fh_connector_t*		fConn;			///< fh_library connector to setup connection
		fh_transport_t*		fTransport;		///< fh_library transport for message IO
		fh_message_t*		fMsg;			///< fh_library message implementation
};

#endif
