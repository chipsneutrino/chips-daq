#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <cstdio>

#include "TimingUtilities.h"

namespace BPT = boost::posix_time;

namespace novadaq {

namespace timeutils {

/**
 * Free function to convert UNIX system time (seconds and nanoseconds)
 * to NOvA base time.
 *
 * @return true if the conversion worked, false if failed
 */
bool convertUnixTimeToNovaTime(struct timespec const& inputUnixTime,
                               uint64_t& outputNovaTime)
{
    if ((unsigned)inputUnixTime.tv_sec < NOVA_EPOCH) {return false;}

    long seconds=inputUnixTime.tv_sec;
    // test in chrono order
    if (seconds > 1341100799) // 23:59:59, Jun 30, 2012
        ++seconds;
    if (seconds > 1435708799) //23:59:59, Jun 30, 2015
    {
        ++seconds;
    }

    if (seconds > 1483228799) //23:59:59, Dec 31, 2016
    {
        ++seconds;
    }

    outputNovaTime =
        ((uint64_t) (seconds - NOVA_EPOCH) * NOVA_TIME_FACTOR) +
        (((uint64_t) inputUnixTime.tv_nsec * NOVA_TIME_FACTOR) / 1000000000);

    return true;
}

/**
 * Free function to convert UNIX system time (seconds and microseconds)
 * to NOvA base time.
 *
 * @return true if the conversion worked, false if failed
 */
bool convertUnixTimeToNovaTime(struct timeval const& inputUnixTime,
                               uint64_t& outputNovaTime)
{
    if ((unsigned)inputUnixTime.tv_sec < NOVA_EPOCH) {return false;}

    // test in chrono order
    time_t adjusted_seconds = inputUnixTime.tv_sec;
    if (adjusted_seconds > 1341100799) // Jun 30 23:59:59 2012 UTC
    {   ++adjusted_seconds;
    }
    if (adjusted_seconds > 1435708799) // Jun 30 23:59:59 2015 UTC
    {   ++adjusted_seconds;
    }

    if (adjusted_seconds > 1483228799) // Dec 31 23:59:59 2016 UTC
    {
      ++adjusted_seconds;
    }

    outputNovaTime =
        ((uint64_t) (adjusted_seconds - NOVA_EPOCH) * NOVA_TIME_FACTOR) +
        (((uint64_t) inputUnixTime.tv_usec * NOVA_TIME_FACTOR) / 1000000);

    return true;
}

/**
 * Free function to convert UNIX system time as a tm struct in UTC
 * to NOvA base time.
 *
 * @param inputUnixTime in a tm struct format (UTC)
 * @param outputNovaTime - nova time in clock ticks relative to epoch
 *
 * @return true if conversion worked, false if failed
 */
bool convertUnixTimeToNovaTime(struct tm inputUnixTime,
                               uint64_t& outputNovaTime)
{
  time_t timeret = timegm(&inputUnixTime);
  if ( timeret < 0 ) return false; // error occurred in timegm
  timeval inputtimeval;
  inputtimeval.tv_sec = timeret;
  inputtimeval.tv_usec = 0;
  return convertUnixTimeToNovaTime(inputtimeval,outputNovaTime);

}

/**
 * Free function to convert UNIX system time (UTC) as a sec,min,hour,mday,mon,year
 * to NOvA base time.
 *
 * The meaning of each input parameter is the same as that in the tm struct:
 * @param sec - seconds after the minute (nominally 0-59, but up to 61 to allow for leap seconds)
 * @param min - minutes after the hour (0-59)
 * @param hour - hours since midnight (0-23) 
 * @param mday - day of the month (1-31)
 * @param mon - months since January (0-11)
 * @param year - years since 1900
 *
 * Output parameter:
 * @param outputNovaTime - nova time in clock ticks relative to epoch
 *
 * @return true if success, else false. 
 */
bool convertUnixTimeToNovaTime(uint16_t sec, uint16_t min, uint16_t hour, uint16_t mday,
                               uint16_t mon, uint32_t year, uint64_t& outputNovaTime) {

  tm timeTM;
  timeTM.tm_sec = sec;
  timeTM.tm_min = min;
  timeTM.tm_hour = hour;
  timeTM.tm_mday = mday;
  timeTM.tm_mon = mon;
  timeTM.tm_year = year;
  return convertUnixTimeToNovaTime(timeTM,outputNovaTime);
  
}

/**
 * Free function to convert NOvA base time to UNIX system time
 * (seconds and microseconds).
 *
 * @return true if the conversion worked, false if failed
 */
bool convertNovaTimeToUnixTime(uint64_t const& inputNovaTime,
                               struct timeval& outputUnixTime)
{
    double doubleTime = (double) inputNovaTime /
        (double) NOVA_TIME_FACTOR;
    time_t time_sec = (time_t) doubleTime;
    outputUnixTime.tv_sec = NOVA_EPOCH + time_sec;
    outputUnixTime.tv_usec = (suseconds_t)
        ((doubleTime - (double)time_sec) * 1000000);
    // test in chrono order
    if (outputUnixTime.tv_sec > 1341100799) // Jun 30 23:59:59 2012 UTC
    {   --outputUnixTime.tv_sec;
    }
    if (outputUnixTime.tv_sec > 1435708799) // Jun 30 23:59:59 2015 UTC
    {   --outputUnixTime.tv_sec;
    }

    if (outputUnixTime.tv_sec > 1483228799) // Dec 31 23:59:59 2016 UTC
    {
      --outputUnixTime.tv_sec;
    }
    return true;
}

/**
 * Free function to convert NOvA base time to UNIX system time
 * (seconds and nanoseconds).
 *
 * @return true if the conversion worked, false if failed
 */
bool convertNovaTimeToUnixTime(uint64_t const& inputNovaTime,
                               struct timespec& outputUnixTime)
{
    double doubleTime = (double) inputNovaTime /
        (double) NOVA_TIME_FACTOR;
    time_t time_sec = (time_t) doubleTime;
    outputUnixTime.tv_sec = NOVA_EPOCH + time_sec;
    outputUnixTime.tv_nsec = (long)
        (inputNovaTime - (time_sec*NOVA_TIME_FACTOR)) * 1000/64;
    // test chronological (start with first nova experienced)
    if (outputUnixTime.tv_sec > 1341100799) // Jun 30 23:59:59 2012 UTC
    {   --outputUnixTime.tv_sec;
    }
    if (outputUnixTime.tv_sec > 1435708799) // Jun 30 23:59:59 2015 UTC
    {   --outputUnixTime.tv_sec;
    }

    if (outputUnixTime.tv_sec > 1483228799) // Dec 31 23:59:59 2016 UTC
    {
      --outputUnixTime.tv_sec;
    }

    return true;
}

/**
 * Free function to convert NOvA base time to UNIX system time (UTC)
 * expressed as a tm struct.
 *
 * @param inputNovaTime - nova time in clock ticks relative to epoch 
 * @param outputUnixTime in a tm struct format 
 *
 * @return true if the conversion worked, else false.
 */
bool convertNovaTimeToUnixTime(uint64_t const& inputNovaTime,
                               struct tm& outputUnixTime)
{
  timeval outputTimeval;
  if ( !convertNovaTimeToUnixTime(inputNovaTime,outputTimeval) ) return false;
  outputUnixTime = *gmtime(&(outputTimeval.tv_sec));  
 
  return true;
}

/**
 * Free function to convert NOvA base time to UNIX system time in (sec,min,hour,...) (UTC)
 * Input parameter:
 * @param inputNovaTime - nova time in clock ticks relative to nova epoch
 * Output parameters:
 * @param sec - seconds after the minute (nominally 0-59, but up to 61 to allow for leap seconds)
 * @param min - minutes after the hour (0-59)
 * @param hour - hours since midnight (0-23) 
 * @param mday - day of the month (1-31)
 * @param mon - months since January (0-11)
 * @param year - years since 1900
 * 
 * @return true if the conversion worked, else false
 */
bool convertNovaTimeToUnixTime(uint64_t const& inputNovaTime,
                               uint16_t& sec, uint16_t& min, uint16_t& hour, uint16_t& mday,
                               uint16_t& mon, uint32_t& year) 
{

  tm outputTimeTM;
  if ( !convertNovaTimeToUnixTime(inputNovaTime,outputTimeTM) ) return false;
  sec = outputTimeTM.tm_sec;
  min = outputTimeTM.tm_min;
  hour = outputTimeTM.tm_hour;
  mday = outputTimeTM.tm_mday;
  mon = outputTimeTM.tm_mon;
  year = outputTimeTM.tm_year;

  return true;

}

/**
 * Free function to return the current time in units of the
 * NOvA base clock.
 *
 * @return true if the fetching of the time worked, false if failed
 */
bool getCurrentNovaTime(uint64_t& outputNovaTime)
{
    struct timeval now;
    if (gettimeofday(&now, 0) != 0) {
        return false;
    }
    return convertUnixTimeToNovaTime(now, outputNovaTime);
}

/**
 * Free function to format a NOvA time as a datetime string.
 */
std::string convertNovaTimeToString(uint64_t const& inputNovaTime)
{
    // deal with whole seconds first
    struct timespec unixTime;
    convertNovaTimeToUnixTime(inputNovaTime, unixTime);
    BPT::ptime posixTime = BPT::from_time_t(unixTime.tv_sec);
    std::string workingString = BPT::to_simple_string(posixTime);

    // now fractional seconds
    /*uint64_t wholePart = ((uint64_t) unixTime.tv_sec - NOVA_EPOCH) *
      NOVA_TIME_FACTOR;*/
    uint64_t wholePart = (inputNovaTime/NOVA_TIME_FACTOR) * NOVA_TIME_FACTOR; // classic integer math
    uint64_t fractionalPart = inputNovaTime - wholePart;
    uint64_t picoSeconds = (uint64_t) (1E12 * fractionalPart / NOVA_TIME_FACTOR);
    char fractionalString[20];
    sprintf(fractionalString, "%012llu", (unsigned long long)picoSeconds);
    workingString.append(".");
    workingString.append(fractionalString);
    workingString.append(" UTC");

    return workingString;
}

/**
 * Free function to format a UNIX timespec as a datetime string.
 */
std::string convertUnixTimeToString(struct timespec const& inputUnixTime)
{
    // deal with whole seconds first
    BPT::ptime posixTime = BPT::from_time_t(inputUnixTime.tv_sec);
    std::string workingString = BPT::to_simple_string(posixTime);

    // now fractional seconds
    char fractionalString[20];
    sprintf(fractionalString, "%09ld", inputUnixTime.tv_nsec);
    workingString.append(".");
    workingString.append(fractionalString);
    workingString.append(" UTC");

    return workingString;
}

/**
 * Free function to format a UNIX timeval as a datetime string.
 */
std::string convertUnixTimeToString(struct timeval const& inputUnixTime)
{
    // deal with whole seconds first
    BPT::ptime posixTime = BPT::from_time_t(inputUnixTime.tv_sec);
    std::string workingString = BPT::to_simple_string(posixTime);

    // now fractional seconds
    char fractionalString[20];
    sprintf(fractionalString, "%06ld", inputUnixTime.tv_usec);
    workingString.append(".");
    workingString.append(fractionalString);
    workingString.append(" UTC");

    return workingString;
}

/* Subtract two timevals, return 0 if positive, 1 if negative */
/* Stolen from: 
   http://www.gnu.org/s/libc/manual/html_node/Elapsed-Time.html */

int timeval_subtract (struct timeval* result, struct timeval* xx, 
                      struct timeval* yy)
{
  // isolate the input varibles: we don't want our carrying later
  // to modify the things being compared.  We only want to 
  // modify the result.   So, create copies to work with.
  struct timeval *x = new struct timeval;
  struct timeval *y = new struct timeval;
  x->tv_sec = xx->tv_sec;  x->tv_usec = xx->tv_usec;
  y->tv_sec = yy->tv_sec;  y->tv_usec = yy->tv_usec;

  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }
     
  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;
     
  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

};  // end of namespace timeutils

};  // end of namespace novadaq
 
