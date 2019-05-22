#include "monitoring_config.h"
#include "packet_generator.h"

namespace po = boost::program_options;

void generatorLoop(PacketGenerator* generator, raw_data_t* data, std::string address, int port, unsigned int num_clbs);

int main(int argc, char* argv[])
{
    // Initialise the elasticsearch interface
    g_elastic.init("daqulator", true, false); // We want log message to be printed to stdout

    // Default settings
    int daq_opt_pot = 56015;
    int daq_mon_pot = 56017;
    std::string daq_address = "localhost";
    std::string configuration_filename("../data/config.opt");
    unsigned int hit_rate = 1; // kHz
    unsigned int deltaTS = 1000;
    unsigned int MTU = 9600;
    unsigned int run_number = 1;

    po::options_description desc("Options");
    desc.add_options()("help,h", "Print this help and exit.")("optport,o",
        po::value<int>(&daq_opt_pot)->default_value(daq_opt_pot),
        "Set the port to send optical data trough.")("monport,m",
        po::value<int>(&daq_mon_pot)->default_value(daq_mon_pot),
        "Set the port to send monitoring data trough.")("address,a",
        po::value<std::string>(&daq_address)->default_value(daq_address),
        "Set the IP address to send data to.")("timeslice,t",
        po::value<unsigned int>(&deltaTS)->default_value(deltaTS),
        "Set the value of the time slice duration in milliseconds.")("hitrate,r",
        po::value<unsigned int>(&hit_rate)->default_value(hit_rate),
        "Set the desired hit rate in kHz.")("runnnumber,n",
        po::value<unsigned int>(&run_number)->default_value(run_number),
        "Set the run number.")("mtu,m",
        po::value<unsigned int>(&MTU)->default_value(MTU),
        "Set the Maximum Transfer Unit (MTU), i.e. the maximum UDP packet size.")("conf,c",
        po::value<std::string>(&configuration_filename)->default_value(configuration_filename),
        "Provide a config file");

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
    } catch (const po::error& e) {
        throw std::runtime_error("DAQulator: Argument error");
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("DAQulator: Argument error");
    }

    MonitoringConfig config(configuration_filename.c_str());
    POMRange_t range = config.getCLBeIDs();
    unsigned int num_clbs = range.size();

    if (range.empty()) {
        throw std::runtime_error("DAQulator: Found no POMS in file");
    }

    // Log the setup to elasticsearch
    std::string setup = "Packet Generators Start (" + daq_address + ",";
    setup += std::to_string(daq_opt_pot) + ",";
    setup += std::to_string(daq_mon_pot) + ",";
    setup += std::to_string(deltaTS) + ",";
    setup += std::to_string(MTU) + ",";
    setup += std::to_string(run_number) + ",";
    setup += std::to_string(hit_rate) + ",";
    setup += configuration_filename + ")";
    g_elastic.log(INFO, setup);

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

    g_elastic.log(INFO, "Packet Generators Stop");
}

void generatorLoop(PacketGenerator* generator, raw_data_t* data, std::string address, int port, unsigned int num_clbs)
{
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
