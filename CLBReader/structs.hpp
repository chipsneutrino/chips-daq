#ifndef CLB_SWISS_KNIFE_STRUCTS_HPP
#define CLB_SWISS_KNIFE_STRUCTS_HPP

#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <arpa/inet.h>
#include "utctime.hh"
#include <ctime>

struct __attribute__((packed)) hit_t
{
  uint8_t channel;
  uint32_t timestamp;
  uint8_t ToT;
};

#define MON_AHRS_VALID 0x4000 // 16384 = 2^14

struct __attribute__((packed)) AHRSData
{
  float yaw;    // < Yaw in deg (Float)
  float pitch;  // < Pitch in deg (Float)
  float roll;   // < Roll in deg (Float)
  float ax;     // < Ax in g (Float)
  float ay;     // < Ay in g (Float)
  float az;     // < Az in g (Float)
  float gx;     // < Gx in deg/sec (Float)
  float gy;     // < Gy in deg/sec (Float)
  float gz;     // < Gz in deg/sec (Float)
  float hx;     // < Hx in gauss (Float)
  float hy;     // < Hy in gauss (Float)
  float hz;     // < Hz in gauss (Float)
};

inline
float ntohl_f(float f)
{
  union {uint32_t integer; float floating;} number;
  number.floating = f;
  number.integer = ntohl(number.integer);
  return number.floating;
}

inline
std::ostream& operator <<(std::ostream& stream, const AHRSData& data)
{
  return stream << "Yaw: "           << ntohl_f(data.yaw)
                << ", Pitch: "       << ntohl_f(data.pitch)
                << ", Roll: "        << ntohl_f(data.roll) << " deg\n"
                << "Acceleration: (" << ntohl_f(data.ax)
                << ", "              << ntohl_f(data.ay)
                << ", "              << ntohl_f(data.az) << ") g\n"
                << "Gyroscope: ("    << ntohl_f(data.gx)
                << ", "              << ntohl_f(data.gy)
                << ", "              << ntohl_f(data.gz) << ") deg/sec\n"
                << "Compass: ("      << ntohl_f(data.hx)
                << ", "              << ntohl_f(data.hy)
                << ", "              << ntohl_f(data.hz) << ") gauss";
}

struct __attribute__((packed)) SCData
{
  uint32_t pad;
  uint32_t valid;
  AHRSData ahrs;
  uint16_t temp;  // temperature in 100th of degrees
  uint16_t humidity; // humidity in 100th RH
};

inline
std::ostream& operator <<(std::ostream& stream, const SCData& data)
{
  return stream << "Validity: " << ntohl(data.valid) << '\n'
                << data.ahrs << '\n'
                << "Temp: " << ntohs(data.temp) / 100. << " Celsius"
                << ", Humidity: " << ntohs(data.humidity) / 100. << " RH";
}

inline
std::ostream& operator <<(std::ostream& stream, const hit_t& hit)
{
  return stream << "C: "
                << std::setfill(' ') << std::setw(2)
                << (unsigned int) hit.channel

                << ", T: "
                << std::setfill(' ') << std::setw(6)
                << ntohl(hit.timestamp)

                << ", ToT: "
                << std::setfill(' ') << std::setw(6)
                << (unsigned int) hit.ToT;
}


struct UTCTime_h : public UTCTime
{
  bool validity;

  UTCTime_h(UTCTime const& ts, bool valid)
    :
    UTCTime(ts),
    validity(valid)
  {}
};

inline
std::ostream& operator <<(std::ostream& stream, const UTCTime_h& timestamp)
{
  const static std::string month[] = {"Jan",
                                      "Feb",
                                      "Mar",
                                      "Apr",
                                      "May",
                                      "Jun",
                                      "Jul",
                                      "Aug",
                                      "Sep",
                                      "Oct",
                                      "Nov",
                                      "Dec"};

  const std::time_t time = timestamp.sec();
  std::tm time_tm;

  if (gmtime_r(&time, &time_tm))
  {
    stream
      << time_tm.tm_year + 1900 << ' '
      << month[time_tm.tm_mon]  << ' '
      << time_tm.tm_mday        << ' '
      << std::setfill('0') << std::setw(2)
      << time_tm.tm_hour        << ':'
      << std::setfill('0') << std::setw(2)
      << time_tm.tm_min         << ':'
      << std::setfill('0') << std::setw(2)
      << time_tm.tm_sec         << " +"
      << std::setfill('0') << std::setw(9)
      << timestamp.tics() * 16
      << "ns GMT";

    if (!timestamp.validity) {
      stream << " (no-sync)";
    }
  } else {
    stream << "error determining the time";
  }

  return stream;
}

#endif // CLB_SWISS_KNIFE_STRUCTS_HPP
