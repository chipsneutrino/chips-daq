/**
 * PacketGenerator - Class to simulate UDP packets from the CLBs
 * 
 * This class simulates the UDP optical packets that are sent from the CLBs
 * to DAQonite on the optical port
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef PACKET_GENERATOR_H_
#define PACKET_GENERATOR_H_

#include "clb_header_structs.h"
#include "clb_data_structs.h"

#include <unistd.h>
#include <string>
#include <fstream>
#include <vector>
#include <sys/time.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <random>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

typedef std::vector<unsigned int> POMRange_t;
typedef std::vector<char> raw_data_t;

#define cool_print(a) { std::cout << #a" = " << a << std::endl; }

class PacketGenerator{
	public:
		PacketGenerator(
			const POMRange_t& dom_range,
			unsigned int time_slice_duration,
			unsigned int run_number,
			unsigned int MTU,
			unsigned int hitR,
			raw_data_t& target,
			unsigned int type
		);

		void getNext(raw_data_t& opt_target);

	private:
		unsigned int m_type;
		unsigned int m_max_seqnumber;
		unsigned int m_delta_ts;
		unsigned int m_payload_size;
		unsigned int m_selected;
		timeval m_tv;
		std::vector<CLBCommonHeader> m_headers;

		// Hit data

		// Monitoring data
		std::default_random_engine m_generator;
		std::normal_distribution<double> m_hit_distribution;
		std::normal_distribution<double> m_t_distribution;
		std::normal_distribution<double> m_h_distribution;
		unsigned int m_mon_hits[32];
		SCData m_mon_data;
};
#endif
