/**
 * PacketGenerator - Class to simulate UDP packets from the CLBs
 */

#include "packet_generator.h"

// Endian swap
#define inplaceEndianSwap32(x) x = ntohl(x);
void swap_endianness(CLBCommonHeader& header)
{
    inplaceEndianSwap32(header.UDPSequenceNumber);
    inplaceEndianSwap32(header.Timestamp.Sec);
    inplaceEndianSwap32(header.Timestamp.Tics);
}

PacketGenerator::PacketGenerator(
    const POMRange_t& pom_range,
    unsigned int time_slice_duration,
    unsigned int run_number,
    unsigned int MTU,
    unsigned int hitR,
    raw_data_t& target,
    unsigned int type)
    : m_type(type)
    , m_delta_ts(time_slice_duration)
    , m_selected((srand(time(0)), rand() % pom_range.size()))
{

    // Set up the CLB packet headers
    m_headers.reserve(pom_range.size());
    for (unsigned int i = 0; i < pom_range.size(); i++) {
        CLBCommonHeader header;
        header.RunNumber = htonl(run_number);
        header.DataType = htonl(m_type);
        header.UDPSequenceNumber = 0;
        header.Timestamp.Sec = time(0);
        header.Timestamp.Tics = 0;
        header.POMIdentifier = htonl(pom_range[i]);
        header.POMStatus1 = 128;
        header.POMStatus2 = 0;
        header.POMStatus3 = 0;
        header.POMStatus4 = 0;
        m_headers.push_back(header);
    }

    if (m_type == ttdc) {
        // max seqnumber  = NPMT * kHz  * Bytes/Hit *   ms TS duration    / (MTU - size of CLB Common Header)
        m_max_seqnumber = 31 * hitR * sizeof(hit_t) * time_slice_duration / (MTU - sizeof(CLBCommonHeader)) + 1;
        m_payload_size = sizeof(hit_t) * ((MTU - sizeof(CLBCommonHeader)) / sizeof(hit_t));
    } else if (m_type == tmch) {
        m_max_seqnumber = 0;
        m_payload_size = (sizeof(int) * 31) + sizeof(SCData);
        m_mon_data.pad = htonl(0);
        m_mon_data.valid = htonl(0);
    }

    m_tv.tv_sec = 0;
    m_tv.tv_usec = 0;
    target.resize(sizeof(CLBCommonHeader) + m_payload_size);

    // Log the setup to elasticsearch
    std::string setup = "Packet Generator (" + std::to_string(m_max_seqnumber) + ",";
    setup += std::to_string(m_payload_size) + ",";
    setup += std::to_string(m_type) + ")";
    g_elastic.log(INFO, setup);
}

void PacketGenerator::getNext(raw_data_t& target)
{
    ++m_selected;
    m_selected %= m_headers.size();

    CLBCommonHeader& common_header = m_headers[m_selected];
    common_header.UDPSequenceNumber = common_header.UDPSequenceNumber + 1;

    if (common_header.UDPSequenceNumber == m_max_seqnumber) {
        common_header.POMStatus2 = 128;
        target.resize(sizeof(common_header));
    } else if (common_header.UDPSequenceNumber == m_max_seqnumber + 1) {
        // Removed check that the last packet was trailer if (isTrailer(common_header))
        common_header.UDPSequenceNumber = 0;
        common_header.POMStatus2 = 0;
        common_header.Timestamp.Tics += 62500 * m_delta_ts;
        if (common_header.Timestamp.Tics >= 62500000) {
            ++common_header.Timestamp.Sec;
            common_header.Timestamp.Tics = 0;
        }
        target.resize(sizeof(CLBCommonHeader) + m_payload_size);
    }

    // Copy the header to the data to send in the packet and swap the endianness
    memcpy(target.data(), &common_header, sizeof(CLBCommonHeader));
    swap_endianness(
        *static_cast<CLBCommonHeader*>(
            static_cast<void*>(
                target.data())));

    if (m_type == tmch) {
        for (int i = 0; i < 31; i++)
            m_mon_hits[i] = htonl(rand() % 10000);
        m_mon_data.temp = (uint16_t)(rand() % 50);
        m_mon_data.humidity = (uint16_t)(rand() % 50);
        memcpy(target.data() + sizeof(CLBCommonHeader), &m_mon_hits, (sizeof(int) * 31));
        memcpy(target.data() + sizeof(CLBCommonHeader) + (sizeof(int) * 31), &m_mon_data, sizeof(SCData));
    }

    // This delays the packets till the next window
    if (common_header.UDPSequenceNumber == 0 && m_selected == 0) {
        int sleep_time = m_delta_ts * 1000;

        if (m_tv.tv_sec) {
            timeval tv;
            gettimeofday(&tv, 0);
            sleep_time -= (tv.tv_sec - m_tv.tv_sec) * 1000000
                + (tv.tv_usec - m_tv.tv_usec);
        }

        if (sleep_time > 0) {
            usleep(sleep_time);
        }

        gettimeofday(&m_tv, 0);
    }
}
