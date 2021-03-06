/**
 * PacketGenerator - Class to simulate UDP packets from the CLBs
 * 
 * This class simulates the UDP optical packets that are sent from the CLBs
 * to DAQonite on the optical port
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#pragma once

#include <clb/data_structs.h>
#include <clb/header_structs.h>
#include "util/daq_config.h"
#include <util/elastic_interface.h>

#include <arpa/inet.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <stdlib.h>
#include <string.h>

#include "TFile.h"
#include "TTree.h"

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

typedef std::vector<unsigned int> POMRange_t;
typedef std::vector<char> raw_data_t;

class PacketGenerator: protected Logging
{
public:
    PacketGenerator(std::string config_file,
                    std::string dataFile,
                    std::string address,
                    int time_slice_duration,
                    int runNum,
                    int MTU,
                    int hitR);

    ~PacketGenerator();

private:
    void workGeneration();

    void opticalSend(raw_data_t data);
    void monitoringSend(raw_data_t data);

    void generate();

    // Config
    DAQConfig fConfig;
    TFile fFile;

    // Output
    boost::asio::io_service fIO_service;
    boost::asio::ip::udp::socket fSock_clb_opt;
    boost::asio::ip::udp::socket fSock_clb_mon;
    boost::asio::ip::udp::udp::endpoint fCLB_opt_endpoint;
    boost::asio::ip::udp::udp::endpoint fCLB_mon_endpoint;

    boost::asio::deadline_timer fTimer;

    // Data Packets
    raw_data_t fCLB_opt_data;
    raw_data_t fCLB_mon_data;

    // Generator Variables
    int fDelta_ts;
    int fMax_packet_hits;
    int fMax_payload_size;
    int fMax_seqnumber;
    std::vector<CLBCommonHeader> fCLB_opt_headers;
    std::vector<CLBCommonHeader> fCLB_mon_headers;
    SCData fMon_data;
    std::vector<std::array<int, 31>> fWindow_hits;
    long fTime_taken;

    // Generator distributions
    std::default_random_engine fGenerator;
    std::normal_distribution<float> fHit_dist;
    std::normal_distribution<float> fTemperature_dist;
    std::normal_distribution<float> fHumidity_dist;
};
