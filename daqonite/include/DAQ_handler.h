/**
 * DAQ_handler - Handler class for combining data streams
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


#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include "TFile.h"
#include "TTree.h"
#include <TApplication.h>
#include <TSystem.h>

#include "DAQ_clb_handler.h"
#include "DAQ_bbb_handler.h"
#include "Monitoring_gui.h"

/// The number of different types of run possible
#define NUMRUNTYPES 4

class DAQ_handler {
	public:

		/**
		 * Create a DAQ_handler
		 * This creates a DAQ_handler, setting up the GUI and local control socket.
		 * Initial work is then added to the IO_service before run() is called to
		 * start to main loop.
		 */
		DAQ_handler(bool collect_clb_optical, bool collect_clb_monitoring,
					bool collect_bbb_optical, bool collect_bbb_monitoring,
					bool gui, int numThreads);

		/// Destroy a DAQ_handler
		~DAQ_handler();

		/**
		 * Setup the data collection and start the IO_service
		 * Opens the output file, sets up the UDP sockets, adds the initial work
		 * to the IO_service needed for data collection.
		 */
		void startRun();

		/**
		 * Stops data collection and saves file
		 * This function stops data collection and writes the TTree's to the file
		 * before saving
		 */		
		void stopRun();

		/**
		 * Exits DAQonite
		 * This function stops the IO_service and terminates the application. It will
		 * call stopRun() first if currently running.
		 */	
		void exit();

		/**
		 * Reads and updates runNumbers.dat
		 * Reads ../data/runNumbers.dat to determine the run number for the given run
		 * type. It then increments this value in the file
		 * 
		 * @return The run number
		 */	
		int getRunAndUpdate();

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

		/**
		 * Handles the local control socket
		 * Handles any commands sent from daq_command over the local control UDP socket.
		 * It reads the input buffer and determines the action to take.
		 * 
		 * @param error Error code from the async_receive()
		 * @param size Signal Number of bytes received
		 */	
		void handleLocalSocket(boost::system::error_code const& error, std::size_t size);

		/// Calls the async_wait() on the signal_set
		void workSignals();

		/// Calls the async_receive() on the local UDP control socket
		void workLocalSocket();

		/**
		 * Keeps the GUI working
		 * Calls the ROOT method ProcessEvents() to process any events for the monitoring
		 * GUI. Need to include a sleep() in order for this to not consume 100% CPU.
		 */	
		void workGui();

	private:
		// DAQ_handler settings
		bool 						fCollect_CLB_optical_data;		///< Should we collect CLB optical data?
		bool 						fCollect_CLB_monitoring_data;	///< Should we collect CLB monitoring data?
		bool 						fCollect_BBB_optical_data;		///< Should we collect BBB optical data?
		bool 						fCollect_BBB_monitoring_data;	///< Should we collect BBB monitoring data?
		bool 						fShow_gui;						///< Should we run the monitoring GUI?
		bool 						fSave_data;						///< Should we save data to .root file?
		int 						fNum_threads;					///< The number of threads to use

		// Mode and running stuff
		bool						fMode;							///< false = Monitoring, True = Running
		unsigned int 				fRun_type;						///< Type of run (data, test, etc...)
		TString 					fFilename;						///< Output file name
		TFile* 						fOutput_file;					///< ROOT output file
		TTree* 						fCLB_optical_tree;				///< ROOT CLB optical output TTree
		TTree* 						fCLB_monitoring_tree;			///< ROOT CLB monitoring output TTree
		TTree* 						fBBB_optical_tree;				///< ROOT BBB optical output TTree
		TTree* 						fBBB_monitoring_tree;			///< ROOT BBB monitoring output TTree

		// IO_service stuff
		boost::asio::io_service* 	fIO_service;					///< BOOST io_service. The heart of everything
		boost::thread_group* 		fThread_group;					///< Group of threads to do the work
		boost::asio::signal_set*	fSignal_set;					///< BOOST signal_set
		udp::socket*				fLocal_socket;					///< Local UDP control socket
		char fBuffer_local[buffer_size] __attribute__((aligned(8)));///< Local socket buffer
		DAQ_clb_handler* 			fCLB_handler;					///< Pointer to CLB_handler
		DAQ_bbb_handler* 			fBBB_handler;					///< Pointer to BBB_handler

		// Monitoring GUI
		Monitoring_gui* 			fDaq_gui;						///< Pointer to the monitoring GUI
};

#endif
