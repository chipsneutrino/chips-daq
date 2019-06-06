/**
 * Program name: DAQulator, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include "packet_generator.h"

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    // Default settings
    std::string address = "localhost";
    std::string config_file("../data/config.opt");
    std::string data_file("../data/testData.root");
    unsigned int hit_rate = 1; // kHz
    unsigned int deltaTS = 10; // ms
    unsigned int MTU = 9600;
    unsigned int run_num = 1;

    bool print_logs = true;
    bool print_debug = false;
    int  index_threads = 10;

    po::options_description desc("Options");
    desc.add_options()("help,h", "DAQulator help...")
    ("address,a", po::value<std::string>(&address)->default_value(address), "SDAQ address")
    ("timeslice,t", po::value<unsigned int>(&deltaTS)->default_value(deltaTS), "Time slice duration (ms)")
    ("hitrate,r", po::value<unsigned int>(&hit_rate)->default_value(hit_rate), "Hit rate (KHz)")
    ("runnum,n", po::value<unsigned int>(&run_num)->default_value(run_num), "Run number")
    ("mtu,m", po::value<unsigned int>(&MTU)->default_value(MTU), "Maximum transfer unit size")
    ("config,c", po::value<std::string>(&config_file)->default_value(config_file), "Config file")
    ("testdata,d", po::value<std::string>(&data_file)->default_value(data_file), "Test data file")
    ("logs", boost::program_options::value<bool>(&print_logs), "Print logs to stdout (true)")
    ("debug", boost::program_options::value<bool>(&print_debug), "Print ElasticInterface debug messages (false)")
    ("threads", boost::program_options::value<int>(&index_threads), "Number of ElasticInterface indexing threads (10)");

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

    g_elastic.init(print_logs, print_debug, index_threads); // Initialise the ElasticInterface

    // Log the setup to elasticsearch
    g_elastic.log(INFO, "DAQulator Start ({},{},{},{},{},{})", address, deltaTS, MTU, run_num, hit_rate, config_file);

    PacketGenerator generator(config_file, data_file, address, deltaTS, run_num, MTU, hit_rate);

    g_elastic.log(INFO, "Packet Generators Stop");
}
