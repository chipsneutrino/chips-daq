#include "packet_generator.h"
#include "monitoring_config.h"

namespace po = boost::program_options;

void generatorLoop(PacketGenerator* generator, raw_data_t* data, std::string address, int port, unsigned int num_clbs);

int main(int argc, char* argv[]) {
	// Default settings
	int daq_opt_pot = 56015;
	int daq_mon_pot = 56017;
	std::string daq_address = "localhost";
	std::string configuration_filename("../data/config.opt");
	unsigned int hit_rate = 1; // kHz
	unsigned int deltaTS = 100;
	unsigned int MTU = 9600;
	unsigned int run_number = 1;

	po::options_description desc("Options");
	desc.add_options()
		("help,h", "Print this help and exit.")
		("optport,o",
			po::value<int>(&daq_opt_pot)->default_value(daq_opt_pot),
			"Set the port to send optical data trough.")
		("monport,m",
			po::value<int>(&daq_mon_pot)->default_value(daq_mon_pot),
			"Set the port to send monitoring data trough.")
		("address,a",
			po::value<std::string>(&daq_address)->default_value(daq_address),
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
		std::cerr << "daqulator: error: " << e.what() << '\n' << desc << std::endl;
		return EXIT_FAILURE;
	} catch (const std::runtime_error& e) {
		std::cerr << "daqulator: error: " << e.what() << '\n' << desc << std::endl;
		return EXIT_FAILURE;
	}

	MonitoringConfig config(configuration_filename.c_str());
	POMRange_t range = config.getCLBeIDs();
	unsigned int num_clbs = range.size();

	if (range.empty()) {
		std::cerr << "daqulator: error: No POM ID range available. Exiting\n";
		return EXIT_FAILURE;
	}

	// Print out the configuration
	std::cout << "daqulator start, with configuration:" << std::endl;;
	cool_print(daq_address);
	cool_print(daq_opt_pot);
	cool_print(daq_mon_pot);
	cool_print(deltaTS);
	cool_print(MTU);
	cool_print(run_number);
	cool_print(hit_rate);
	std::cout << "POM IDs: [";
	for (unsigned int i = 0; i < num_clbs; ++i) std::cout << range[i] << ' ';
	std::cout << "]" << std::endl << std::endl;

	// Create optical packet generator
	raw_data_t opt_data;
	PacketGenerator opt_generator(range, deltaTS, run_number, MTU, hit_rate, opt_data, ttdc);

	// Create moitoring packet generator
	raw_data_t mon_data;
	PacketGenerator mon_generator(range, deltaTS, run_number, MTU, hit_rate, mon_data, tmch);

	// Start the two generators on seperate threads
    boost::thread optThread(generatorLoop, &opt_generator, &opt_data, daq_address, daq_opt_pot, num_clbs);
	boost::thread monThread(generatorLoop, &mon_generator, &mon_data, daq_address, daq_mon_pot, num_clbs);

    optThread.join();
	monThread.join();
}

void generatorLoop(PacketGenerator* generator, raw_data_t* data, std::string address, int port, unsigned int num_clbs) {
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
		if (((++windows) % 500) == 0) {
			 std::cout << "Thread (" << boost::this_thread::get_id() << ") windows -> " << windows << std::endl;
		}
	}	
}
