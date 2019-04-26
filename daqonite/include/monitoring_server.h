/**
 * MonitoringServer - Stores histograms and provides the ROOT THttpServer 
 * 
 * Allows for the viewing of histograms and plots via a web server
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef MONITORING_SERVER_H_
#define MONITORING_SERVER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/lexical_cast.hpp>

#include <THttpServer.h>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TNtuple.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TROOT.h>

#include "clb_header_structs.h"
#include "clb_data_structs.h"
#include "daq_logging.h"

#define PROCESSRATE 20
#define UPDATERATE 1000

#define BUFFERSIZE 10000

#define GENERALPORT 56019
#define CLBMONPORT 56017
#define BBBMONPORT 56018

const static ssize_t clb_minimum_size = sizeof(CLBCommonHeader) + (sizeof(int)*31);
const static ssize_t clb_max_size = sizeof(CLBCommonHeader) + (sizeof(int)*31) + sizeof(SCData);

class MonitoringServer {
    public:
        // Create a MonitoringServer
        MonitoringServer(std::string config_file, float generalFrac,
                         float clbFrac, float bbbFrac);

        // Destroy a MonitoringServer
        ~MonitoringServer();

        // Generate filename
        std::string generateFilename();

        // Setup the TTree
        void setupTree();

        // Work/Handle the general input socket
        void workGeneralSocket();
        void handleGeneralSocket(boost::system::error_code const& error, std::size_t size);

        // Work/Handle the CLB monitoring socket
        void workCLBSocket();
        void handleCLBSocket(boost::system::error_code const& error, std::size_t size);

        // Work/Handle the BBB monitoring socket
        void workBBBSocket();
        void handleBBBSocket(boost::system::error_code const& error, std::size_t size);

        // Work/Handle the ROOT Process to keep HTTPServer responsive
        void workROOTProcess();
        void handleROOTProcess();

        // Work/Handle updating the plots
        void workPlotUpdate();
        void handlePlotUpdate();

        // Work/Handle signals
        void workSignals();
        void handleSignals(boost::system::error_code const& error, int signum);      

    private:

        THttpServer fServer;                                            ///< THttpServer used for viewing the plots
        TFile*      fFile;                                              ///< Output ROOT file for saving monitoring data

        boost::asio::io_service fIO_service;                            ///< The BOOST io_service
        boost::asio::signal_set fSignal_set;                            ///< Signal set to deal with process killing
        
        // General Socket
        boost::asio::ip::udp::socket fGeneral_socket;                   ///< Socket to send general monitoring data to
        char fGeneral_buffer[BUFFERSIZE] __attribute__((aligned(8)));   ///< General monitoring socket buffer
        float fGeneral_frac;                                            ///< Fraction of general monitoring data to keep

        // CLB Socket
        boost::asio::ip::udp::socket fCLB_socket;                       ///< Socket to send CLB monitoring data to
        char fCLB_buffer[BUFFERSIZE] __attribute__((aligned(8)));       ///< CLB monitoring socket buffer
        float fCLB_frac;                                                ///< Fraction of CLB monitoring data to keep
        TTree* fCLB_tree;                                               ///< ROOT TTree to store CLB monitoring data
		uint32_t 	fCLB_pom_id;		                                ///< Mon CLB: Header POM ID (4 bytes)
		uint32_t 	fCLB_timestamp_s;                                   ///< Mon CLB: Header timestamp (4 bytes)
		uint32_t 	fCLB_pad;   		                                ///< Mon CLB: Header Pad (4 bytes)
		uint32_t 	fCLB_valid; 		                                ///< Mon CLB: Header Valid (4 bytes)
		uint16_t 	fCLB_temperature; 	                                ///< Mon CLB: Temperature data (2 bytes)
		uint16_t 	fCLB_humidity;	                                    ///< Mon CLB: Humidity data (2 bytes)
		uint32_t 	fCLB_hits[30];  	                                ///< Mon CLB: Channel Hits (4 bytes)

        // BBB Socket
        boost::asio::ip::udp::socket fBBB_socket;                       ///< Socket to send BBB monitoring data to
        char fBBB_buffer[BUFFERSIZE] __attribute__((aligned(8)));       ///< BBB monitoring socket buffer
        float fBBB_frac;                                                ///< Fraction of BBB monitoring data to keep

        // ROOT Process
        boost::asio::deadline_timer fProcess_timer;                     ///< Timer for ROOT process calls

        // Plot Update
        boost::asio::deadline_timer fUpdate_timer;                      ///< Timer for plot update calls
};

#endif