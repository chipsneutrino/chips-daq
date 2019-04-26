/**
 * Program name: DAQuardian, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "monitoring_server.h"

int main(int argc, char* argv[]) {

    init_daq_logging();

    std::string config_file	= "../data/config.opt";

    BOOST_LOG_TRIVIAL(info) << "DAQuardian: Starting MonitoringServer using " << config_file;
    MonitoringServer server(config_file, 1.0, 1.0, 1.0);
    BOOST_LOG_TRIVIAL(info) << "DAQuardian: MonitoringServer stopped";
}
