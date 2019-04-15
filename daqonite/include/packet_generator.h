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

#include <vector>
#include <sys/time.h>

typedef std::vector<unsigned int> POMRange_t;

typedef std::vector<char> raw_data_t;

class PacketGenerator
{
	unsigned int m_max_seqnumber;
	unsigned int m_delta_ts;
	unsigned int m_payload_size;
	unsigned int m_selected;
	timeval m_tv;
	std::vector<CLBCommonHeader> m_headers;

	public:

	PacketGenerator(
		const POMRange_t& dom_range,
		unsigned int time_slice_duration,
		unsigned int run_number,
		unsigned int MTU,
		unsigned int hitR,
		raw_data_t& target
	);

	void getNext(raw_data_t& target);

};
#endif
