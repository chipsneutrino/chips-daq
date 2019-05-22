#ifndef _NOVADAQ_NOVATIME_UTILITIES_H
#define _NOVADAQ_NOVATIME_UTILITIES_H

#include <cstdint>
#include <sys/time.h>
#include <time.h>
#include <string>

namespace novadaq {

namespace timeutils {

// *time_t* of start of Nova epoch, 01-Jan-2010 00:00:00, UTC
// This is the value subtracted from the UNIX time_t, timeval or timespec
// seconds field. Since "novatime" does not have leap seconds and
// "unixtime" does, leap seconds which happen _after_ the nova epoch will
// need to be factored in to get the correct novatime which corresponds to
// the time after the leap second.
const uint32_t NOVA_EPOCH = 1262304000;

// conversion factor (related to clock frequency)
const uint64_t NOVA_TIME_FACTOR = 64000000;

bool convertUnixTimeToNovaTime(struct timespec const& inputUnixTime,
                               uint64_t& outputNovaTime);

bool convertUnixTimeToNovaTime(struct timeval const& inputUnixTime,
                               uint64_t& outputNovaTime);

bool convertUnixTimeToNovaTime(struct tm inputUnixTime,
                               uint64_t& outputNovaTime);

bool convertUnixTimeToNovaTime(uint16_t sec, uint16_t min, uint16_t hour, uint16_t mday,
                               uint16_t mon, uint32_t year, uint64_t& outputNovaTime);

bool convertNovaTimeToUnixTime(uint64_t const& inputNovaTime,
                               struct timespec& outputUnixTime);

bool convertNovaTimeToUnixTime(uint64_t const& inputNovaTime,
                               struct timeval& outputUnixTime);

bool convertNovaTimeToUnixTime(uint64_t const& inputNovaTime,
                               struct tm& outputUnixTime);

bool convertNovaTimeToUnixTime(uint64_t const& inputNovaTime,
                               uint16_t& sec, uint16_t& min, uint16_t& hour, uint16_t &mday,
                               uint16_t& mon, uint32_t& year);

bool getCurrentNovaTime(uint64_t& outputNovaTime);

//uint64_t convertUnixTimeToNovaTime(struct timeval const& inputUnixTime);

//struct timeval convertNovaTimeToUnixTime(uint64_t const& inputNovaTime);

std::string convertNovaTimeToString(uint64_t const& inputNovaTime);

std::string convertUnixTimeToString(struct timespec const& inputNovaTime);

std::string convertUnixTimeToString(struct timeval const& inputNovaTime);

int timeval_subtract (struct timeval * result,  struct timeval *x,  
                      struct timeval *y);

};  // end of namespace timeutils

};  // end of namespace novadaq

#endif
