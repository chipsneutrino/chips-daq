#ifndef __FRAME_GENERATOR_HH
#define __FRAME_GENERATOR_HH

#include "clb_header_structs.h"

#include <vector>
#include <sys/time.h>

typedef std::vector<unsigned int> POMRange_t;

typedef std::vector<char> raw_data_t;

class FrameGenerator
{
	unsigned int m_max_seqnumber;
	unsigned int m_delta_ts;
	unsigned int m_payload_size;
	unsigned int m_selected;
	timeval m_tv;
	std::vector<CLBCommonHeader> m_headers;

	public:

	FrameGenerator(
		const POMRange_t& dom_range,
		unsigned int time_slice_duration,
		unsigned int run_number,
		unsigned int MTU,
		unsigned int hitR
	);

	void getNext(raw_data_t& target);

};
#endif
