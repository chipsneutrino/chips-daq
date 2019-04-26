/**
 * Program name: DAQuardian, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "monitoring_server.h"

int main(int argc, char* argv[]) {

    std::string config_file	= "../data/config.opt";
    MonitoringServer server(config_file, 1.0, 1.0, 1.0);
}
