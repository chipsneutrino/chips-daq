#ifndef DATAQUEUE_DATAFORMATS_CLB_COMMON_HEADER_HH
#define DATAQUEUE_DATAFORMATS_CLB_COMMON_HEADER_HH

#include <ostream>
#include <stdint.h>
#include <cassert>
#include <cstdlib>
#include <arpa/inet.h>

#include "utctime.hh"

struct CLBCommonHeader
{
  uint32_t DataType;
  uint32_t RunNumber;
  uint32_t UDPSequenceNumber;
  UTCTime Timestamp;
  uint32_t DOMIdentifier;
  uint32_t DOMStatus1;
  uint32_t DOMStatus2;
  uint32_t DOMStatus3;
  uint32_t DOMStatus4;

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

  uint32_t domIdentifier() const
  {
    return ntohl(DOMIdentifier);
  }

  uint32_t domStatus(int n = 1) const
  {
    switch (n) {
      case 1:
        return ntohl(DOMStatus1);
      case 2:
        return ntohl(DOMStatus2);
      case 3:
        return ntohl(DOMStatus3);
      case 4:
        return ntohl(DOMStatus4);
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
                <<  "DOMIdentifier:     " << header.domIdentifier()     << '\n'
                <<  "DOMStatus1:        " << header.domStatus(1)        << '\n'
                <<  "DOMStatus2:        " << header.domStatus(2)        << '\n'
                <<  "DOMStatus3:        " << header.domStatus(3)        << '\n'
                <<  "DOMStatus4:        " << header.domStatus(4);
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
  value += header.domIdentifier();
  return value;
}

inline
uint32_t dom_id(frame_idx_t idx)
{
  frame_idx_t const mask = 0x00000000FFFFFFFF;

  return idx & mask;
}

inline
bool validTimeStamp(CLBCommonHeader const& header)
{
  const static uint32_t mask = 0x80000000;

  return header.domStatus() & mask;
}

inline
bool isTrailer(CLBCommonHeader const& header)
{
  const static uint32_t mask = 0x80000000;

  return header.domStatus(2) & mask;
}

#endif // DATAQUEUE_DATAFORMATS_CLB_COMMON_HEADER_HH
