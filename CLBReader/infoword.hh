#ifndef __INFO_WORD_HH
#define __INFO_WORD_HH

#include <ostream>

#include <stdint.h>

static
inline unsigned int roundUpPow2(unsigned int n, unsigned int multiple)
{
  return (n + multiple - 1) & ~(multiple - 1);
}

struct __attribute__ ((__packed__)) InfoWord
{
  uint8_t head;
  uint8_t SamplingRate;
  uint32_t TimeInfo;

  unsigned int ChannelMask() const
  {
    const static uint8_t mask = 0x60;
    return (mask & head) >> 5;
  }

  unsigned int Amplitude() const
  {
    const static uint8_t mask = 0x18;
    return (mask & head) >> 3;
  }

  bool Mark() const
  {
    const static uint8_t mask = 0x80;
    return mask & head;
  }

  unsigned int amplitude() const
  {
    switch (Amplitude())
    {
      case 1: return 16;
      case 2: return 24;
    }
    return 12;
  }

  unsigned int timeInfo() const
  {
    return ntohl(TimeInfo);
  }

  double samplingRate() const
  {
    const static double conv_factor = 1e6 / 128;
    return SamplingRate * conv_factor;
  }

  unsigned int audioWordSize() const
  {
    unsigned int sample_size_bit = amplitude();

    if (!ChannelMask())       // 0 ==> 2 channels, else only one
      sample_size_bit <<= 1;  // fast double the conteined value

    const unsigned int size_bit = sample_size_bit + 8;

    return roundUpPow2(size_bit, 16) >> 3; // return, in Bytes, the
                                           // minimum multiple of 16
                                           // bit greater then size_bit
  }

};

inline
std::ostream& operator <<(std::ostream& stream, const InfoWord& iw)
{
  return stream
      << "Is an InfoWord:        " << iw.Mark()                  << '\n'
      << "ChannelMask:           " << iw.ChannelMask()           << '\n'
      << "Amplitude:             " << iw.Amplitude()             << '\n'
      << "Amplitude (human):     " << iw.amplitude() << " bit"   << '\n'
      << "Sampling rate:         " << (uint32_t) iw.SamplingRate << '\n'
      << "Sampling rate (human): " << iw.samplingRate() << " Hz" << '\n'
      << "TimeInfo:              " << iw.timeInfo();
}

inline
bool is_infoword(const void* const data)
{
  const static unsigned char mask = 0x80;
  const unsigned char* const p = static_cast<const unsigned char* const>(
      data);

  return (*p & mask);
}
#endif
