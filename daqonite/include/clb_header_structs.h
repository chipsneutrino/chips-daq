/**
 * CLB Header Structs - Data structures for the CLB packet header
 * 
 * This contains all the data structures needed to decode the CLB UDP header
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#pragma once

#include "clb_data_structs.h"

#define MONI 0x1
#define ACOU 0x2
#define OPTO 0x4
#define AUTO (MONI | ACOU | OPTO)

const static unsigned int ttdc = 1414808643;
const static unsigned int taes = 1413563731;
const static unsigned int tmch = 1414349640;

/// Struct describing the CLB header
struct CLBCommonHeader
{
    uint32_t DataType;          ///< Type of data optical, acoustic, monitoring
    uint32_t RunNumber;         ///< Run number
    uint32_t UDPSequenceNumber; ///< Packet sequence number to check for dropped packets
    UTCTime Timestamp; ///< UTC timestamp, with seconds and tics
    uint32_t POMIdentifier; ///< POM identification number
    uint32_t POMStatus1; ///< Status indicator 1
    uint32_t POMStatus2; ///< Status indicator 2
    uint32_t POMStatus3; ///< Status indicator 3
    uint32_t POMStatus4; ///< Status indicator 4

    uint32_t dataType() const
    {
        return ntohl(DataType);
    }

    uint32_t runNumber() const
    {
        return ntohl(RunNumber);
    }

    uint32_t udpSequenceNumber() const
    {
        return ntohl(UDPSequenceNumber);
    }

    const UTCTime& timeStamp() const
    {
        return Timestamp;
    }

    uint32_t pomIdentifier() const
    {
        return ntohl(POMIdentifier);
    }

    uint32_t pomStatus(int n = 1) const
    {
        switch (n) {
        case 1:
            return ntohl(POMStatus1);
        case 2:
            return ntohl(POMStatus2);
        case 3:
            return ntohl(POMStatus3);
        case 4:
            return ntohl(POMStatus4);
        default:
            assert(!"Programming error: invalid field requested");
            abort();
        }
    }

    std::bitset<32> statusBitset(int n = 1) const
    {
        switch (n)
        {
        case 1:
            return std::bitset<32>(ntohl(POMStatus1));
        case 2:
            return std::bitset<32>(ntohl(POMStatus2));
        case 3:
            return std::bitset<32>(ntohl(POMStatus3));
        case 4:
            return std::bitset<32>(ntohl(POMStatus4));
        default:
            assert(!"Programming error: invalid field requested");
            abort();
        }
    }
};

/// Get the data type from the CLB header
inline std::pair<int, std::string> getType(CLBCommonHeader const &header)
{
    const static std::pair<int, std::string> unknown = std::make_pair(-1, "unknown");
    const static std::pair<int, std::string> acoustic = std::make_pair(ACOU, "acoustic data");
    const static std::pair<int, std::string> optical = std::make_pair(OPTO, "optical data");
    const static std::pair<int, std::string> monitoring = std::make_pair(MONI, "monitoring data");

    if (header.dataType() == tmch) {
        return monitoring;
    } else if (header.dataType() == ttdc) {
        return optical;
    } else if (header.dataType() == taes) {
        return acoustic;
    }
    return unknown;
}

/// << print operator for the CLB UDP header
inline std::ostream &operator<<(std::ostream &stream, const CLBCommonHeader &header)
{
    return stream << "DataType:          " << header.dataType()
                  << " (" << getType(header).second << ")\n"
                  << "RunNumber:         " << header.runNumber() << '\n'
                  << "UDPSequenceNumber: " << header.udpSequenceNumber() << '\n'
                  << "Timestamp:         " << header.timeStamp()
                  << "POMIdentifier:     " << header.pomIdentifier() << '\n'
                  << "POMStatus1:        " << header.statusBitset(1).to_string() << '\n'
                  << "POMStatus2:        " << header.statusBitset(2).to_string() << '\n'
                  << "POMStatus3:        " << header.statusBitset(3).to_string() << '\n'
                  << "POMStatus4:        " << header.statusBitset(4).to_string() << '\n';
}

/// Is the timestamp valid?
inline bool validTimeStamp(CLBCommonHeader const& header)
{
    const static uint32_t mask = 0x80000000;
    return header.pomStatus() & mask;
}

/// Is it a trailer?
inline bool isTrailer(CLBCommonHeader const& header)
{
    const static uint32_t mask = 0x80000000;
    return header.pomStatus(2) & mask;
}

/// Has a veto been activated?
inline bool vetoActivated(CLBCommonHeader const &header)
{
    const static uint32_t mask = 0x7FFFFFFF;
    return header.pomStatus() & mask;
}

/// Is a FIFO full?
inline bool fullFIFO(CLBCommonHeader const &header)
{
    const static uint32_t mask = 0x7FFFFFFF;
    return header.pomStatus(2) & mask;
}
