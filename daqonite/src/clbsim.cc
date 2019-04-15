#include <iostream>
#include "frame_generator.h"
#include "monitoring_config.h"
#include <unistd.h>
#include <string>
#include <fstream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#define cool_print(a) { std::cout << #a" = " << a << std::endl; }

int main(int argc, char* argv[])
{
	int dq_port = 4000;
	unsigned int hit_rate = 10; // kHz

	std::string dq_address;

	std::string input_filename("");
	std::string configuration_filename("../data/config.opt");

	unsigned int deltaTS = 100;

	unsigned int MTU = 1500;

	unsigned int run_number = 24;

	po::options_description desc("Options");
	desc.add_options()
		("help,h",     "Print this help and exit.")
		("port,p",
			po::value<int>(&dq_port)->required(),
			"Set the port to send data trough.")
		("address,a",
			po::value<std::string>(&dq_address)->required(),
			"Set the IP address to send data to.")
		("timeslice,t",
			po::value<unsigned int>(&deltaTS)->default_value(deltaTS),
			"Set the value of the time slice duration in milliseconds.")
		("hitrate,r",
			po::value<unsigned int>(&hit_rate)->default_value(hit_rate),
			"Set the desired hit rate in kHz.")
		("runnnumber,n",
			po::value<unsigned int>(&run_number)->default_value(run_number),
			"Set the run number.")
		("mtu,m",
			po::value<unsigned int>(&MTU)->default_value(MTU),
			"Set the Maximum Transfer Unit (MTU), i.e. the maximum UDP packet size.")
		("conf,c",
			po::value<std::string>(&configuration_filename)->default_value(configuration_filename),
			"Provide a config file");

	try {
		po::variables_map vm;
		po::store(
			po::command_line_parser(argc, argv).options(desc).run(),
			vm);

		if (vm.count("help")) {
		std::cout << desc << std::endl;
		return EXIT_SUCCESS;
		}

		po::notify(vm);
	} catch (const po::error& e) {
		std::cerr << "CLBsimu: Error: " << e.what() << '\n'
				<< desc << std::endl;
		return EXIT_FAILURE;
	} catch (const std::runtime_error& e) {
		std::cerr << "CLBsimu: Error: " << e.what() << '\n'
				<< desc << std::endl;
		return EXIT_FAILURE;
	}

	MonitoringConfig config(configuration_filename.c_str());
	POMRange_t range = config.getCLBeIDs();

	if (range.empty()) {
		std::cerr << "FATAL: No POM ID range available. Exiting\n";
		return EXIT_FAILURE;
	}

	std::cout << "Program UP!\n";
	std::cout << "here is the configuration:\n";
	cool_print(dq_address);
	cool_print(dq_port);
	cool_print(deltaTS);
	cool_print(MTU);
	cool_print(run_number);
	cool_print(hit_rate);

	std::cout << "POM IDs:\n";

	for (unsigned int i = 0; i < range.size(); ++i) {
		std::cout << range[i] << ' ';
	}
	std::cout << '\n';

	raw_data_t data;

	FrameGenerator generator(range, deltaTS, run_number, MTU, hit_rate, data);

	boost::asio::io_service service;

	boost::asio::ip::udp::socket sock(service, boost::asio::ip::udp::udp::v4());

	boost::asio::ip::udp::udp::resolver resolver(service);

	boost::asio::ip::udp::udp::resolver::query query(
		boost::asio::ip::udp::udp::v4(),
		dq_address,
		boost::lexical_cast<std::string>(dq_port));

	boost::asio::ip::udp::udp::endpoint const destination = *resolver.resolve(query);

	while (true) {
		for (unsigned int i = 0; i < range.size(); i++) {
			generator.getNext(data);

			sock.send_to(boost::asio::buffer(data), destination);
		}
	}
}
