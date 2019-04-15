#include "frame_generator.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>

#define inplaceEndianSwap32(x) x = ntohl(x);

void swap_endianness(CLBCommonHeader& header)
{
// Endian swap
inplaceEndianSwap32(header.UDPSequenceNumber);

// Endian swap time
inplaceEndianSwap32(header.Timestamp.Sec);
inplaceEndianSwap32(header.Timestamp.Tics)
}

FrameGenerator::FrameGenerator(
	const POMRange_t& dom_range,
	unsigned int time_slice_duration,
	unsigned int run_number,
	unsigned int MTU,
	unsigned int hitR) :
	m_delta_ts(time_slice_duration),
	m_selected((srand(time(0)), rand() % dom_range.size()))
{
	m_headers.reserve(dom_range.size());

	for (unsigned int i = 0; i < dom_range.size(); i++) {
		CLBCommonHeader header;
		header.RunNumber = htonl(run_number);
		header.DataType = htonl(ttdc);
		header.UDPSequenceNumber = 0;
		header.Timestamp.Sec = time(0);
		header.Timestamp.Tics = 0;
		header.POMIdentifier = htonl(dom_range[i]);
		header.POMStatus1 = 128;
		header.POMStatus2 = 0;
		header.POMStatus3 = 0;
		header.POMStatus4 = 0;

		m_headers.push_back(header);
	}

	// max seqnumber  = NPMT * kHz  * Bytes/Hit *   ms TS duration    / (MTU - size of CLB Common Header)
	m_max_seqnumber =  31  * hitR *     6     * time_slice_duration / (MTU - sizeof(CLBCommonHeader)) + 1;
	m_payload_size = 6 * ((MTU - sizeof(CLBCommonHeader)) / 6);
	m_tv.tv_sec  = 0;
	m_tv.tv_usec = 0;
	std::cout << "m_max_seqnumber -> " << m_max_seqnumber << std::endl;
}

void FrameGenerator::getNext(raw_data_t& target)
{
	++m_selected;
	m_selected %= m_headers.size();

	CLBCommonHeader& common_header = m_headers[m_selected];

	common_header.UDPSequenceNumber = common_header.UDPSequenceNumber + 1;

	target.resize(sizeof(CLBCommonHeader) + m_payload_size);

	if (common_header.UDPSequenceNumber == m_max_seqnumber) {
		common_header.POMStatus2 = 128;

		//std::cout << "resize target 1 -> " << sizeof(common_header) << std::endl;
		target.resize(sizeof(common_header));
	} else if (common_header.UDPSequenceNumber == m_max_seqnumber + 1) {
		if (isTrailer(common_header)) {
		common_header.UDPSequenceNumber = 0;
		common_header.POMStatus2 = 0;
		common_header.Timestamp.Tics += 62500 * m_delta_ts;
		if (common_header.Timestamp.Tics >= 62500000) {
			++common_header.Timestamp.Sec;
			common_header.Timestamp.Tics = 0;
		}
		} else {
		assert(!"Programming error: UDPSequenceNumber and trailer not respected.");
		}

		//std::cout << "resize target 2 -> " << sizeof(CLBCommonHeader) + m_payload_size << std::endl;
		target.resize(sizeof(CLBCommonHeader) + m_payload_size);
	}

	memcpy(target.data(), &common_header, sizeof(CLBCommonHeader));

	swap_endianness(
		*static_cast<CLBCommonHeader*>(
			static_cast<void*>(
				target.data()
			)
		)
	);

	if (common_header.UDPSequenceNumber == 0 && m_selected == 0) {
		int sleep_time = m_delta_ts * 1000;

		if (m_tv.tv_sec) {
		timeval tv;
		gettimeofday(&tv, 0);
		sleep_time -=
			(tv.tv_sec - m_tv.tv_sec) * 1000000
			+ (tv.tv_usec - m_tv.tv_usec);
		}

		if (sleep_time > 0) {
		usleep(sleep_time);
		}

		gettimeofday(&m_tv, 0);
	}

	//std::cout << common_header.POMIdentifier << std::endl;
	//std::cout << common_header.UDPSequenceNumber << std::endl;
}
