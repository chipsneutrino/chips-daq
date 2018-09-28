/*
 * clb_header_structs.h
 * Data structures for the CLB header
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#ifndef CLB_HEADER_STRUCTS_H
#define CLB_HEADER_STRUCTS_H

#include "clb_data_structs.h"

struct CLBCommonHeader
{
  uint32_t DataType;
  uint32_t RunNumber;
  uint32_t UDPSequenceNumber;
  UTCTime Timestamp;
  uint32_t POMIdentifier;
  uint32_t POMStatus1;
  uint32_t POMStatus2;
  uint32_t POMStatus3;
  uint32_t POMStatus4;

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

  UTCTime timeStamp() const
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
};

inline std::ostream& operator <<(std::ostream& stream, const CLBCommonHeader& header)
{
  return stream <<  "DataType:          " << header.dataType()          << '\n'
                <<  "RunNumber:         " << header.runNumber()         << '\n'
                <<  "UDPSequenceNumber: " << header.udpSequenceNumber() << '\n'
                <<  "Timestamp:       \n" << header.timeStamp()         << '\n'
                <<  "POMIdentifier:     " << header.pomIdentifier()     << '\n'
                <<  "POMStatus1:        " << header.pomStatus(1)        << '\n'
                <<  "POMStatus2:        " << header.pomStatus(2)        << '\n'
                <<  "POMStatus3:        " << header.pomStatus(3)        << '\n'
                <<  "POMStatus4:        " << header.pomStatus(4);
}

typedef uint64_t frame_idx_t;

inline
int32_t seq_number(
    CLBCommonHeader const& header,
    uint64_t start_run_ms,
    int ts_duration_ms)
{
  return (header.timeStamp().inMilliSeconds() - start_run_ms) / ts_duration_ms;
}

inline
frame_idx_t data2idx(
    CLBCommonHeader const& header,
    uint64_t start_run_ms,
    int ts_duration_ms)
{
  frame_idx_t value = seq_number(header, start_run_ms, ts_duration_ms);
  value <<= 32;
  value += header.pomIdentifier();
  return value;
}

inline
uint32_t pom_id(frame_idx_t idx)
{
  frame_idx_t const mask = 0x00000000FFFFFFFF;

  return idx & mask;
}

inline
bool validTimeStamp(CLBCommonHeader const& header)
{
  const static uint32_t mask = 0x80000000;

  return header.pomStatus() & mask;
}

inline
bool isTrailer(CLBCommonHeader const& header)
{
  const static uint32_t mask = 0x80000000;

  return header.pomStatus(2) & mask;
}

#endif // CLB_HEADER_STRUCTS_H
