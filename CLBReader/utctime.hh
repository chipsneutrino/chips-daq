#ifndef __UTCTIME_HH
#define __UTCTIME_HH

#include <ostream>
#include <stdint.h>
#include <arpa/inet.h>

struct UTCTime
{
  uint32_t Sec;
  uint32_t Tics;

  uint32_t sec() const
  {
    return ntohl(Sec);
  }

  uint32_t tics() const
  {
    return ntohl(Tics);
  }

  uint64_t inMilliSeconds() const
  {
    return uint64_t(sec()) * 1000 + uint64_t(tics()) / 62500;
  }

};

inline std::ostream& operator <<(std::ostream& stream, const UTCTime& t)
{
  return stream << "Seconds: " << t.sec() << '\n'
                << "Tics:    " << t.tics();
}

#endif
