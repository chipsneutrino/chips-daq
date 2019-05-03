/**
 * Program name: DAQuardian, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "monitoring_server.h"
#include "elastic_interface.h"

int main(int argc, char* argv[]) {

    g_elastic.init("daquardian", true);

    std::string config_file	= "../data/config.opt";

    g_elastic.log(INFO, "Starting MonitoringServer using " + config_file);
    MonitoringServer server(config_file, 1.0, 1.0);
    g_elastic.log(INFO, "MonitoringServer stopped");

    return 0;
}
