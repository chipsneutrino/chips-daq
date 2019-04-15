/**
 * DAQHandler - Handler class for combining data streams
 * 
 * This is the main class that deals with the DAQ across all stream
 * It holds a CLB_handler and BBB_handler object which deal with the 
 * individual streams data collection. It controls the IO_service 
 * which provides the backbone to the entire DAQonite program.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef DAQ_HANDLER_H_
#define DAQ_HANDLER_H_

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

#include "TFile.h"
#include "TTree.h"
#include <TApplication.h>
#include <TSystem.h>

#include "clb_opt_handler.h"
#include "clb_mon_handler.h"
#include "bbb_handler.h"
#include "monitoring_gui.h"
#include "data_handler.h"

/// The number of different types of run possible
#define GUIROOTRATE 10
#define GUIUPDATERATE 1000

/// Buffer size in bytes for local data
const static size_t buffer_size_local = 10000;

class DAQHandler {
	public:
		/**
		 * Create a DAQHandler
		 * This creates a DAQHandler, setting up the GUI and local control socket.
		 * Initial work is then added to the IO_service before run() is called to
		 * start to main loop.
		 */
		DAQHandler(bool collect_clb_data, bool collect_bbb_data,
				   bool gui, int numThreads, std::string configFile);

		/// Destroy a DAQHandler
		~DAQHandler();

	private:
		/**
		 * Binded to thread creation
		 * Allows us to modify how the thread operates and what it does
		 */	
		void ioServiceThread();

		/**
		 * Handles UNIX signals
		 * Handles the interupts from UNIX signals. Currently only ctrl-c is defined
		 * which calls exit() on the application.
		 * 
		 * @param error Signals error code
		 * @param signum Signal number
		 */	
		void handleSignals(boost::system::error_code const& error, int signum);

		/// Calls the async_wait() on the signal_set
		void workSignals();

		/**
		 * Handles the local control socket
		 * Handles any commands sent from daq_command over the local control UDP socket.
		 * It reads the input buffer and determines the action to take.
		 * 
		 * @param error Error code from the async_receive()
		 * @param size Signal Number of bytes received
		 */	
		void handleLocalSocket(boost::system::error_code const& error, std::size_t size);

		/// Calls the async_receive() on the local UDP control socket
		void workLocalSocket();

		/**
		 * Handles the updating of the GUI
		 * Updates the plots in the GUI to show the most recent monitoring info
		 */			
		void handleGuiUpdate();

		/// Calls the async_wait() on the update timer
		void workGuiUpdate();

		/**
		 * Keeps the ROOT GUI processing events (clicks etc...)
		 * Calls the ProcessEvents() method from ROOT
		 */	
		void workGuiEvents();

		// Settings
		bool 							fCollect_clb_data;			///< Should we collect CLB optical data?
		bool 							fCollect_bbb_data;			///< Should we collect CLB monitoring data?
		bool 							fShow_gui;					///< Should we run the monitoring GUI?
		int 							fNum_threads;				///< The number of threads to use

		// Mode (monitoring vs data taking)
		bool							fMode;						///< false = Monitoring, True = Running

		// IO_service stuff
		boost::asio::io_service* 		fIO_service;				///< BOOST io_service. The heart of everything
		boost::thread_group* 			fThread_group;				///< Group of threads to do the work
		boost::asio::signal_set*		fSignal_set;				///< BOOST signal_set
		udp::socket*					fLocal_socket;				///< Local UDP control socket
		char fBuffer_local[buffer_size_local] __attribute__((aligned(8)));///< Local socket buffer
		DataHandler						fData_handler;				///< DataHandler object
		CLBOptHandler* 					fCLB_opt_handler;			///< Pointer to CLBOptHandler
		CLBMonHandler* 					fCLB_mon_handler;			///< Pointer to CLBMonHandler
		BBBHandler* 					fBBB_handler;				///< Pointer to BBBHandler

		// Monitoring GUI
		boost::asio::deadline_timer*	fGui_event_timer;			///< Boost GUI ROOT event timer
		boost::asio::deadline_timer*	fGui_update_timer;			///< Boost GUI update timer
		MonitoringGui*	 				fDaq_gui;					///< Pointer to the MonitoringGui
};

#endif
