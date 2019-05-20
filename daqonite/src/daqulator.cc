/**
 * Program name: DAQulator, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "packet_generator.h"
#include "daq_config.h"

namespace po = boost::program_options;

int main(int argc, char* argv[]) {

    // Initialise the elasticsearch interface
    g_elastic.init("daqulator", true, false, 5);  // We want log message to be printed to stdout

	// Default settings
	std::string address = "localhost";
	std::string config_file("../data/config.opt");
	std::string data_file("../data/testData.root");
	unsigned int hit_rate = 1; // kHz
	unsigned int deltaTS = 10; // ms
	unsigned int MTU = 9600;
	unsigned int run_num = 1;

	po::options_description desc("Options");
	desc.add_options()
		("help,h", "DAQulator help...")
		("address,a", po::value<std::string>(&address)->default_value(address), "SDAQ address")
		("timeslice,t", po::value<unsigned int>(&deltaTS)->default_value(deltaTS), "Time slice duration (ms)")
		("hitrate,r", po::value<unsigned int>(&hit_rate)->default_value(hit_rate), "Hit rate (KHz)")
		("runnum,n", po::value<unsigned int>(&run_num)->default_value(run_num), "Run number")
		("mtu,m", po::value<unsigned int>(&MTU)->default_value(MTU), "Maximum transfer unit size")
		("config,c", po::value<std::string>(&config_file)->default_value(config_file), "Config file")
		("testdata,d", po::value<std::string>(&data_file)->default_value(data_file), "Test data file");

	try {
		po::variables_map vm;
		po::store(
			po::command_line_parser(argc, argv).options(desc).run(),
			vm);

		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return 0;
		}

		po::notify(vm);
	} catch (const po::error& e) { throw std::runtime_error("DAQulator: Argument error"); 
	} catch (const std::runtime_error& e) { throw std::runtime_error("DAQulator: Argument error"); }

    // Log the setup to elasticsearch
    std::string setup = "DAQulator Start (" + address + ",";
	setup += std::to_string(deltaTS) + ",";
	setup += std::to_string(MTU) + ",";
	setup += std::to_string(run_num) + ",";
	setup += std::to_string(hit_rate) + ",";
	setup += config_file + ")"; 
    g_elastic.log(INFO, setup);

	PacketGenerator generator(config_file, data_file, address, deltaTS,	run_num, MTU, hit_rate);

	g_elastic.log(INFO, "Packet Generators Stop");
}
