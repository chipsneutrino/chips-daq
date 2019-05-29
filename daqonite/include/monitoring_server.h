/**
 * MonitoringServer - Stores histograms and provides the ROOT THttpServer 
 * 
 * Allows for the viewing of histograms and plots via a web server
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#pragma once

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH1F.h>
#include <THttpServer.h>
#include <TNtuple.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TTree.h>

#include "clb_data_structs.h"
#include "clb_header_structs.h"
#include "elastic_interface.h"

#define BUFFERSIZE 10000

#define CLBMONPORT 56017
#define BBBMONPORT 56018

const static std::size_t clb_mon_size = 240;

class MonitoringServer
{
public:
    /// Create a MonitoringServer
    MonitoringServer(std::string config_file,
                     bool save_elastic, bool save_file, bool show_gui,
                     float clbFrac, float bbbFrac);

    /// Destroy a MonitoringServer
    ~MonitoringServer();

    /**
		 * Generate a filename for the ROOT output file
		 * The name is based on the current time and date
		 */
    std::string generateFilename();

    /**
		 * Setup the ROOT file TTree with the needed branches
		 * This should deal with both CLB and BBB setup
		 */
    void setupTree();

    // Work/Handle the CLB monitoring socket
    void workCLBSocket();
    void handleCLBSocket(boost::system::error_code const &error, std::size_t size);

    // Work/Handle the BBB monitoring socket
    void workBBBSocket();
    void handleBBBSocket(boost::system::error_code const &error, std::size_t size);

    // Work/Handle signals
    void workSignals();
    void handleSignals(boost::system::error_code const &error, int signum);

private:
    // settings
    bool fSave_elastic;                     ///< Save data to elasticsearch
    bool fSave_file;                        ///< Save data to ROOT file
    bool fShow_gui;                         ///< Show old ROOT monitoring GUI

    // io_service
    boost::asio::io_service fIO_service;    ///< The BOOST io_service
    boost::asio::signal_set fSignal_set;    ///< Signal set to deal with process killing

    // ROOT file
    TFile *fFile;                           ///< Output ROOT file for saving monitoring data
    TTree *fCLB_tree;                       ///< ROOT TTree to store CLB monitoring data

    // CLB Socket
    boost::asio::ip::udp::socket fCLB_socket;                 ///< Socket to send CLB monitoring data to
    char fCLB_buffer[BUFFERSIZE] __attribute__((aligned(8))); ///< CLB monitoring socket buffer
    float fCLB_frac;                                          ///< Fraction of CLB monitoring data to keep

    int fCLB_run_num;                       ///< Mon CLB: Header Run Num (4 bytes)
    int fCLB_pom_id;                        ///< Mon CLB: Header POM ID (4 bytes)
    long fCLB_timestamp;                    ///< Mon CLB: Header timestamp in ms (8 bytes)
    int fCLB_temperature;                   ///< Mon CLB: Temperature data (2 bytes)
    int fCLB_humidity;                      ///< Mon CLB: Humidity data (2 bytes)
    float fCLB_rates[30];                   ///< Mon CLB: Channel Hits (4 bytes)
    bool fRate_veto;                        ///< Mon CLB: Was a high rate veto present (bool)

    // BBB Socket
    boost::asio::ip::udp::socket fBBB_socket;                 ///< Socket to send BBB monitoring data to
    char fBBB_buffer[BUFFERSIZE] __attribute__((aligned(8))); ///< BBB monitoring socket buffer
    float fBBB_frac;                                          ///< Fraction of BBB monitoring data to keep
};
