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

#define PROCESSRATE 20
#define UPDATERATE 1000
#define BUFFERSIZE 10000
#define PORT 8081

class MonitoringServer {
    public:
        // Create a MonitoringServer
        MonitoringServer(std::string config_file);

        // Destroy a MonitoringServer
        ~MonitoringServer();

        // Work/Handle the local socket
        void handleSocket(boost::system::error_code const& error, std::size_t size);
        void workSocket();

        // Work/Handle ROOT events to keep HTTPServer Working
        void workProcess();
        void handleUpdate();
        void workUpdate();

    private:
        boost::asio::io_service fIO_service;                    ///< The BOOST io_service
        boost::asio::deadline_timer fProcess_timer;             ///< Timer to handle server requests
        boost::asio::deadline_timer fUpdate_timer;              ///< Timer to update the plots
        boost::asio::ip::udp::socket fSocket;                   ///< Socket application send monitoring data to
        char fBuffer[BUFFERSIZE] __attribute__((aligned(8)));   ///< Buffer to incoming socket messages
        THttpServer server;                                     ///< THttpServer used to view plots

        TFile* file;                                            ///< TFile to open for monitoring
        TTree* tree;                                            ///< TTree to use within file                              
        TCanvas c1;                                             ///< Canvas to display plot
        Int_t first;                                            ///< Keep track of entries in TTree
};

#endif