/**

 Program name: CLBReader

 Description:

 This program is a modification of the CLBSwissKnife...
 Original Author: Carmelo Pellegrino
 E-mail: carmelo.pellegrino@bo.infn.it
 Date:   27 October 2014

 Modified Version Author: Josh Tingey
 E-mail: j.tingey.16@ucl.ac.uk
 Date:   18 September 2018

 While running it listens to a specific UDP port and collects
 data sent to it by CHIPS CLB's

 The user can specify the port to bind and if monitoring data
 is required...

 - if the port is specified using the "-p" or "--port"
 options the type is automatically determined as
 data comes;

 - if the port is specified using one of the other
 options ("-o/--optical",
 "-m/--monitoring"), the program is forced to parse
 the incoming data with the specified data type;
 with these options it is not mandatory to specify a
 port number, since a default value is pre-defined.

 With the "-f/--dump-file" option is also possible to
 specify a destination file in which the incoming
 data will be stored for offline analysis.

 Es: CLBReader -m -f test.bin

 With the "--map" option you can specify a map file to be used to match
 the DOM IDs of incoming data.
 The DOM map file is an ASCII file reporting assignments of DOM IDs
 or MAC addresses with names. Each assignment must be separated by
 a semicolon. An example of map file follows:

 08:00:30:BE:B7:A2 = CLB19;
 817805038 = CLB13;

 By default, CLBReader will try to load the file ~/.doms.csk as a
 map file. You can just write the information on your setup to this
 file and let CLBSK use it without specifying any "--map" option.

 Use
 $ CLBReader -h
 for a detailed help.
 */

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <sys/ioctl.h>
#include <signal.h>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "clb_common_header.hh"
#include "infoword.hh"
#include "structs.hh"
#include "version.hh"

int get_terminal_width() {
	static bool const is_a_tty = isatty(1);

	if (!is_a_tty) {
		return 80;
	} else {
		struct winsize sz;

		ioctl(1, TIOCGWINSZ, &sz);
		return sz.ws_col;
	}
}

#define MONI 0x1
#define ACOU 0x2
#define OPTO 0x4
#define AUTO (MONI | ACOU | OPTO)

namespace po = boost::program_options;
namespace asio = boost::asio;
using boost::asio::ip::udp;

// TODO: use the CLBDataGram instead of a buffer

const static size_t buffer_size = 10000;

const static unsigned int default_opto_port = 56015;
const static unsigned int default_acou_port = 56016;
const static unsigned int default_moni_port = 56017;

const static unsigned int ttdc = 1414808643;
const static unsigned int taes = 1413563731;
const static unsigned int tmch = 1414349640;

std::pair<int, std::string> const& getType(CLBCommonHeader const& header);

void print_header(CLBCommonHeader const& header,
		std::map<uint32_t, std::string> const& name_map);

void print_optical_data(const char* const buffer, ssize_t buffer_size,
		int max_col);

void print_monitoring_data(const char* const buffer, ssize_t buffer_size,
		int max_col);

void handle_signal(boost::asio::signal_set& set, int& terminal_width,
		boost::system::error_code const& error, int signum);

class DataHandler {
	boost::asio::ip::udp::socket& m_socket;
	int const m_datatype;

	// Input
	char* m_buffer;
	std::size_t const m_buffer_size;

	// Output
	int& m_terminal_width;
	std::map<uint32_t, std::string> const& m_dom_names_map;

	// Optional features
	bool m_dump;
	std::ofstream& m_output_file;


	std::size_t m_counter;

	void handle(boost::system::error_code const& error, std::size_t size) {
		if (!error) {
			const std::size_t linewidth =
					m_terminal_width > 2 ? m_terminal_width - 2 : 1;

			std::cout << ' ' << std::string(linewidth, '=');
			std::cout << "\nCounter: " << ++m_counter << '\n';
			std::cout << "Size of the datagram: " << size << '\n';

			if (m_buffer_size - sizeof(CLBCommonHeader) < 0) {
				std::cerr << "Invalid packet size: " << size
						<< " < size of CLBCommonHeader ("
						<< sizeof(CLBCommonHeader) << "). Skipping.\n";
				work();
				return;
			}

			CLBCommonHeader const
					& header =
							*static_cast<CLBCommonHeader const*> (static_cast<void const*> (m_buffer));

			int running_type = m_datatype;

			if (m_datatype == AUTO) {
				std::pair<int, std::string> const& type = getType(header);
				running_type = type.first;
				std::cout << "Data type (automatically determined): "
						<< type.second << std::endl;
			}

			print_header(header, m_dom_names_map);

			if (running_type == OPTO) {
				print_optical_data(m_buffer, size, m_terminal_width);
			} else if (running_type == MONI) {
				print_monitoring_data(m_buffer, size, m_terminal_width);
			}

			if (m_dump) {
				uint32_t const s = size;
				if (!m_output_file.write(
						static_cast<char const* const > (static_cast<void const* const > (&s)),
						sizeof(s)).write(m_buffer, s)) {
					throw std::runtime_error(
							"CLBReader: Error: Unable to write to file.");
				}
			}

			work();
		}
	}

	DataHandler(DataHandler const&); // non-copyable
	DataHandler& operator =(DataHandler const&); // non-copyable

public:

	DataHandler(boost::asio::ip::udp::socket& socket, int data_type,
			char* buffer, std::size_t buffer_size, int& terminal_width,
			std::map<uint32_t, std::string> const& dom_names_map, bool dump,
			std::ofstream& output_file) :
		m_socket(socket), m_datatype(data_type), m_buffer(buffer),
				m_buffer_size(buffer_size), m_terminal_width(terminal_width),
				m_dom_names_map(dom_names_map), m_dump(dump), m_output_file(
						output_file), m_counter(0) {
	}

	void work() {
		m_socket.async_receive(boost::asio::buffer(m_buffer, m_buffer_size),
				boost::bind(&DataHandler::handle, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
};

uint32_t getDomId(std::string const& s_domid);
std::map<uint32_t, std::string> load_dom_map(std::istream& map);

int main(int argc, char* argv[]) {
	bool dump = false;
	int type = 0;

	std::string dom_names = std::getenv("HOME") + std::string("/.doms.csk");
	std::map<uint32_t, std::string> dom_names_map;

	std::ofstream output_file;

	unsigned int port = 0;

	std::string filename;

	po::options_description desc("Options");
	desc.add_options()("help,h", "Print this help and exit.")("version,v",
			"Print the version and exit.")(
			"optical,o",
			po::value<unsigned int>(&port) ->implicit_value(default_opto_port) ->value_name(
					"port"),
			"Set the expected data type to optical. If a port is not \
provided the default one is used (56015).")(
			"monitoring,m",
			po::value<unsigned int>(&port) ->implicit_value(default_moni_port) ->value_name(
					"port"),
			"Set the expected data type to monitoring. If a port is \
not provided the default one is used (56017).")(
			"port,p", po::value<unsigned int>(&port)->value_name("port"),
			"Listen on the specified port, automatically determining the \
data type.")(
			"dumpfile,f", po::value<std::string>(&filename)->value_name(
					"filename"), "Dumps the acquired data to a file.")("map",
			po::value<std::string>(&dom_names)->value_name("filename"),
			"File that specifies a conversion from DOM IDs to a human readable name.");

	std::string templ;

	try {
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return EXIT_SUCCESS;
		}

		if (vm.count("version")) {
			std::cout << clbreader::version::v() << std::endl;
			return EXIT_SUCCESS;
		}

		po::notify(vm);

		unsigned int counter = 0;
		if (vm.count("port")) {
			type = AUTO;
			++counter;
		}
		if (vm.count("optical")) {
			type = OPTO;
			++counter;
		}
		if (vm.count("monitoring")) {
			type = MONI;
			++counter;
		}

		if (counter != 1) {
			if (counter) {
				throw std::runtime_error("More than one port provided.");
			} else {
				throw std::runtime_error("You must specify at least one port.");
			}
		}

		if (vm.count("map")) {
			std::ifstream map(dom_names.c_str());

			if (!map) {
				throw std::runtime_error(
						std::string("Error reading map file: ")
								+ std::strerror(errno));
			}

			dom_names_map = load_dom_map(map);
		} else {
			std::ifstream map(dom_names.c_str());

			dom_names_map = load_dom_map(map);
		}

		if (vm.count("dumpfile")) {
			output_file.open(filename.c_str(), std::ios::binary);
			if (!output_file) {
				throw std::runtime_error("Unable to open file " + filename
						+ " for writing: " + strerror(errno));
			}
			dump = true;
		}
	} catch (const po::error& e) {
		std::cerr << "CLBReader: Error: " << e.what() << '\n' << desc
				<< std::endl;
		return EXIT_FAILURE;
	} catch (const std::runtime_error& e) {
		std::cerr << "CLBReader: Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	// Initialise the terminal width
	int terminal_width = get_terminal_width();

	std::cout << "CLBReader - by Josh Tingey MSci, JoshTingeyGuiGod.Josh" << std::endl;

	std::cout << "Listening port: " << port << '\n' << "Data type: ";

	if (type == OPTO) {
		std::cout << "Optical\n";
	} else if (type == MONI) {
		std::cout << "Monitoring\n";
	} else {
		std::cout << "Automatically determined\n";
	}

	asio::io_service io_service;

	udp::socket socket(io_service, udp::endpoint(udp::v4(), port));
	udp::socket::receive_buffer_size option(33554432);
	socket.set_option(option);

	char buffer[buffer_size] __attribute__((aligned(8)));

	boost::asio::signal_set signals_handler(io_service, SIGWINCH, SIGINT);

	signals_handler.async_wait(boost::bind(handle_signal, boost::ref(
			signals_handler), boost::ref(terminal_width),
			boost::asio::placeholders::error,
			boost::asio::placeholders::signal_number));

	DataHandler dh(socket, type, buffer, buffer_size, terminal_width,
			dom_names_map, dump, output_file);

	dh.work();

	io_service.run();

}

std::pair<int, std::string> const& getType(CLBCommonHeader const& header) {
	const static std::pair<int, std::string> unknown = std::make_pair(-1,
			"unknown");
	const static std::pair<int, std::string> acoustic = std::make_pair(ACOU,
			"acoustic data");
	const static std::pair<int, std::string> optical = std::make_pair(OPTO,
			"optical data");
	const static std::pair<int, std::string> monitoring = std::make_pair(MONI,
			"monitoring data");

	if (header.dataType() == tmch) {
		return monitoring;
	}

	if (header.dataType() == ttdc) {
		return optical;
	}

	if (header.dataType() == taes) {
		return acoustic;
	}

	return unknown;
}

struct DOMID_h {
	uint32_t m_val;
	DOMID_h(uint32_t val) :
		m_val(val) {
	}
};

std::ostream& operator <<(std::ostream& stream, const DOMID_h& domid) {
	/*
	 * The MAC address of a WR node starts with 08:00:30.
	 * The DOMID is defined by the MAC address removing the initial 08:00.
	 */

	std::ostringstream oss("0800", std::ostringstream::ate);
	oss << std::hex << domid.m_val;
	if (oss.tellp() != 12) {
		return stream << "undefined";
	}

	std::string const no = oss.str();
	std::size_t const s = no.size();
	for (std::size_t i = 0; i < s; i += 2) {
		stream << char(toupper(no[i])) << char(toupper(no[i + 1])) << (i != s
				- 2 ? ":" : "");
	}
	return stream;
}

void print_header(CLBCommonHeader const& header,
		std::map<uint32_t, std::string> const& name_map) {
	bool const valid = validTimeStamp(header);
	bool const trailer = isTrailer(header);

	std::string name("");
	if (name_map.size()) {
		std::map<uint32_t, std::string>::const_iterator const it =
				name_map.find(header.domIdentifier());

		if (it != name_map.end()) {
			name = " - " + it->second;
		} else {
			name = " - unknown";
		}
	}

	std::cout << "DataType:          " << header.dataType() << '\n'
			<< "RunNumber:         " << header.runNumber() << '\n'
			<< "UDPSequenceNumber: " << header.udpSequenceNumber() << '\n'

	<< "Timestamp:\n" << "          Seconds: " << header.timeStamp().sec()
			<< '\n' << "          Tics:    " << header.timeStamp().tics()
			<< '\n' << "          " << UTCTime_h(header.timeStamp(), valid)
			<< '\n'

	<< "DOMIdentifier:     " << header.domIdentifier() << " (MAC: " << DOMID_h(
			header.domIdentifier()) << name << ')' << '\n'
			<< "DOMStatus 1:       " << header.domStatus(1) << '\n'
			<< "DOMStatus 2:       " << header.domStatus(2);

	if (trailer && header.dataType() == ttdc) {
		std::cout << " (trailer)\n";
	} else {
		std::cout << '\n';
	}

	std::cout << "DOMStatus 3:       " << header.domStatus(3) << '\n'
			<< "DOMStatus 4:       " << header.domStatus(4) << std::endl;
}

void print_optical_data(const char* const buffer, ssize_t buffer_size,
		int max_col) {
	const unsigned int nhits = (buffer_size - sizeof(CLBCommonHeader))
			/ sizeof(hit_t);

	std::cout << "Number of hits: " << nhits << '\n';

	if (nhits) {
		const int printing = 20 > nhits ? nhits : 20;
		const unsigned int n = max_col > 37 ? max_col / 37 : 1;

		for (int i = 0; i < printing; ++i) {
			const hit_t
					* const hit =
							static_cast<const hit_t* const > (static_cast<const void* const > (buffer
									+ sizeof(CLBCommonHeader) + i
									* sizeof(hit_t)));

			std::cout << "Hit" << std::setfill('0') << std::setw(2) << i
					<< ": " << *hit << ' ';

			if ((i + 1) % n == 0) {
				std::cout << '\n';
			} else {
				std::cout << "| ";
			}
		}
	}
	std::cout << '\n';
}

void print_monitoring_data(const char* const buffer, ssize_t buffer_size,
		int max_col) {
	const unsigned int n = max_col > 14 ? max_col / 14 : 1;

	for (int i = 0; i < 31; ++i) {
		const uint32_t
				* const field =
						static_cast<const uint32_t* const > (static_cast<const void* const > (buffer
								+ sizeof(CLBCommonHeader) + i * 4));

		std::cout << "CH" << std::setfill('0') << std::setw(2) << i << ": "
				<< std::setfill(' ') << std::setw(6) << htonl(*field) << "  ";

		if ((i + 1) % n == 0) {
			std::cout << '\n';
		}
	}
	std::cout << '\n';

	const ssize_t minimum_size = sizeof(CLBCommonHeader) + sizeof(int) * 31;

	if (buffer_size > minimum_size) {
		std::cout << "SlowControl data:\n"
				<< *static_cast<const SCData* const > (static_cast<const void* const > (buffer
						+ minimum_size)) << '\n';
	}
}

uint32_t getDomId(std::string const& s_domid) {
	if (s_domid.find(":") == std::string::npos) {
		return boost::lexical_cast<uint32_t>(s_domid);
	} else {
		boost::char_separator<char> const args_sep(":");
		boost::tokenizer<boost::char_separator<char> > tok(s_domid, args_sep);

		std::stringstream ss;
		int count = 0;
		BOOST_FOREACH(std::string byte, tok)
					{
						if (byte.size() != 2) {
							throw std::runtime_error(
									"Error: malformed MAC address");
						}

						if (count > 1) {
							ss << byte;
						} else {
							if (count == 0 && byte == "08") {
							} else if (count == 1 && byte == "00") {
							} else {
								throw std::runtime_error(
										"Error: the provided MAC address is not a CLB MAC.");
							}
						}
						++count;
					}

		return strtol(ss.str().c_str(), 0, 16);
	}
}

std::map<uint32_t, std::string> load_dom_map(std::istream& map) {
	boost::char_separator<char> const args_sep(";");
	boost::char_separator<char> const fields_sep("=");
	boost::char_separator<char> const white_spaces("\t ");

	std::map<uint32_t, std::string> dom_map;

	while (map.peek() != EOF) {
		std::string line;
		getline(map, line);

		std::string::size_type const pound = line.find('#');

		if (pound != std::string::npos) {
			line = line.substr(0, pound);
		}

		boost::algorithm::trim(line);
		boost::tokenizer<boost::char_separator<char> > tok(line, args_sep);
		BOOST_FOREACH(std::string s, tok)
					{
						boost::algorithm::trim(s);
						boost::tokenizer<boost::char_separator<char> > fields(
								s, fields_sep);
						typedef boost::tokenizer<boost::char_separator<char> >::iterator
								Iter;

						Iter it = fields.begin();

						std::string s_domid = *it;
						boost::algorithm::trim(s_domid);
						uint32_t const domid = getDomId(s_domid);

						++it;
						if (it != fields.end()) {
							std::string name = *it;
							boost::algorithm::trim(name);

							dom_map.insert(std::make_pair(domid, name));
						} else {
							throw std::runtime_error(
									"Error: malformed assignment");
						}
					}
	}

	return dom_map;
}

void handle_signal(boost::asio::signal_set& set, int& terminal_width,
		boost::system::error_code const& error, int signum) {
	if (!error) {
		if (signum == SIGINT) {
			set.get_io_service().stop();
			if (isatty(1)) {
				std::cout
						<< "\033]2;thank you for flying with CLBReader!\007";
				std::cout << "CLBReader End..." << std::endl;
			}
			return;
		} else if (signum == SIGWINCH) {
			terminal_width = get_terminal_width();
		}

		set.async_wait(boost::bind(handle_signal, boost::ref(set), boost::ref(
				terminal_width), boost::asio::placeholders::error,
				boost::asio::placeholders::signal_number));
	}
}
