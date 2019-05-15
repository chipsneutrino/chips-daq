/**
 * Program name: DAQulator, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "packet_generator.h"
#include "daq_config.h"

namespace po = boost::program_options;

void generatorLoop(PacketGenerator* generator, raw_data_t* data, std::string address, 
				   int port, unsigned int num_clbs, unsigned int packetType);

int main(int argc, char* argv[]) {

    // Initialise the elasticsearch interface
    g_elastic.init("daqulator", true, false, 5);  // We want log message to be printed to stdout

	// Default settings
	int opt_port = 56015;
	int mon_port = 56017;
	std::string address = "localhost";
	std::string config_file("../data/config.opt");
	unsigned int hit_rate = 1; // kHz
	unsigned int deltaTS = 100;
	unsigned int MTU = 9600;
	unsigned int run_num = 1;

	po::options_description desc("Options");
	desc.add_options()
		("help,h", "DAQulator help...")
		("optport,o", po::value<int>(&opt_port)->default_value(opt_port), "Optical port")
		("monport,m", po::value<int>(&mon_port)->default_value(mon_port), "Monitoring port")
		("address,a", po::value<std::string>(&address)->default_value(address), "SDAQ address")
		("timeslice,t", po::value<unsigned int>(&deltaTS)->default_value(deltaTS), "Time slice duration (ms)")
		("hitrate,r", po::value<unsigned int>(&hit_rate)->default_value(hit_rate), "Hit rate (KHz)")
		("runnum,n", po::value<unsigned int>(&run_num)->default_value(run_num), "Run number")
		("mtu,m", po::value<unsigned int>(&MTU)->default_value(MTU), "Maximum transfer unit size")
		("config,c", po::value<std::string>(&config_file)->default_value(config_file), "Config file");

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

	DAQConfig config(config_file.c_str());
	POMRange_t range = config.getCLBeIDs();
	unsigned int num_clbs = range.size();

	config.printConfig();

	if (range.empty()) { throw std::runtime_error("DAQulator: Found no POMS in file"); }

    // Log the setup to elasticsearch
    std::string setup = "Packet Generators Start (" + address + ",";
	setup += std::to_string(opt_port) + ",";
	setup += std::to_string(mon_port) + ",";
	setup += std::to_string(deltaTS) + ",";
	setup += std::to_string(MTU) + ",";
	setup += std::to_string(run_num) + ",";
	setup += std::to_string(hit_rate) + ",";
	setup += config_file + ")"; 
    g_elastic.log(INFO, setup);

	// Create optical packet generator
	raw_data_t opt_data;
	PacketGenerator opt_generator(range, deltaTS, run_num, MTU, hit_rate, opt_data, ttdc);

	// Create moitoring packet generator
	raw_data_t mon_data;
	PacketGenerator mon_generator(range, deltaTS, run_num, MTU, hit_rate, mon_data, tmch);

	// Start the two generators on seperate threads
    boost::thread optThread(generatorLoop, &opt_generator, &opt_data, address, opt_port, num_clbs, ttdc);
	boost::thread monThread(generatorLoop, &mon_generator, &mon_data, address, mon_port, num_clbs, tmch);

    optThread.join();
	monThread.join();

	g_elastic.log(INFO, "Packet Generators Stop");
}

void generatorLoop(PacketGenerator* generator, raw_data_t* data, std::string address, 
				   int port, unsigned int num_clbs, unsigned int type) {
	// Setup the socket to send generate packets to
	boost::asio::io_service service;
	boost::asio::ip::udp::socket sock(service, boost::asio::ip::udp::udp::v4());
	boost::asio::ip::udp::udp::resolver resolver(service);
	boost::asio::ip::udp::udp::resolver::query query(boost::asio::ip::udp::udp::v4(),
													 address,
													 boost::lexical_cast<std::string>(port));
	boost::asio::ip::udp::udp::endpoint const destination = *resolver.resolve(query);

	// Start sending simulated data packets
	int windows = 0;
	while (true) {
		for (unsigned int i = 0; i < num_clbs; i++) {
			generator->getNext(*data);
			sock.send_to(boost::asio::buffer(*data), destination);
		}
		if (((++windows) % 1000) == 0) {
			g_elastic.log(INFO, "Packet Generator ("+std::to_string(type)+") Windows -> "+std::to_string(windows));
		}
	}	
}
