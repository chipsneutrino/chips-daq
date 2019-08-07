/**
 * MonitoringHandler - Reads monitoring packets and forwards them to elasticsearch or file
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
#include <boost/thread.hpp>

#include <TFile.h>
#include <TTree.h>

#include <clb/data_structs.h>
#include <clb/header_structs.h>
#include "util/command_receiver.h"
#include <util/elastic_interface.h>

#define BUFFERSIZE 10000

#define CLBMONPORT 56017
#define BBBMONPORT 56018

const static std::size_t clb_mon_size = 240;

class MonitoringHandler : public CommandHandler {
public:
    /// Create a MonitoringHandler
    MonitoringHandler(std::string config_file, bool save_elastic,
                     bool save_file, float sample_frac);

    /// Destroy a MonitoringHandler
    virtual ~MonitoringHandler() = default;

    // for safety, no copy- or move-semantics
    MonitoringHandler(const MonitoringHandler& other) = delete;
    MonitoringHandler(MonitoringHandler&& other) = delete;

    MonitoringHandler& operator=(const MonitoringHandler& other) = delete;
    MonitoringHandler& operator=(MonitoringHandler&& other) = delete;

    virtual void handleConfigCommand(std::string config_file) override;
    virtual void handleStartDataCommand() override;
    virtual void handleStopDataCommand() override;
    virtual void handleStartRunCommand(RunType which) override;
    virtual void handleStopRunCommand() override;
    virtual void handleExitCommand() override;

    void run();

    bool getMode()
    {
        return mode_;
    }

private:

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

    /// run() io_service in a new thread
    void runThread();

    // settings
    bool save_elastic_; ///< Save data to elasticsearch
    bool save_file_;    ///< Save data to ROOT file
    float sample_frac_; ///< Fraction of monitoring data to keep
    int n_threads_;     ///< Number of threads to use for monitoring

    // Running mode
    bool mode_; ///< false = Not Running, True = Running
    RunType run_type_;

    // io_service
    std::shared_ptr<boost::asio::io_service> io_service_; ///< BOOST io_service. The heart of everything
    boost::thread_group thread_group_;   ///< Group of threads to read packets

    // ROOT file
    TFile *file_;     ///< Output ROOT file for saving monitoring data
    TTree *clb_tree_; ///< ROOT TTree to store CLB monitoring data

    // CLB Socket
    boost::asio::ip::udp::socket clb_socket_;                 ///< Socket to send CLB monitoring data to
    char clb_buffer_[BUFFERSIZE] __attribute__((aligned(8))); ///< CLB monitoring socket buffer

    pom_data pom_data_;
    channel_data channel_data_;

    // BBB Socket
    boost::asio::ip::udp::socket bbb_socket_;                 ///< Socket to send BBB monitoring data to
    char bbb_buffer_[BUFFERSIZE] __attribute__((aligned(8))); ///< BBB monitoring socket buffer
};
