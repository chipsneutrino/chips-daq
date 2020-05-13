/**
 * Timestamp - All timestamp formats used by CHIPS.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <cstdint>
#include <ostream>

#include <boost/date_time.hpp>

/// Time measured in relation to the TAI system, stored with up to ~1 ns precision.
/// Here, TAI is a high-accuracy monotonic (need better term) time system measured
/// to .1 microsecond resolution by atomic clocks around the world. Not adjusted
/// to earth rotation. For more see http://www.bipm.fr/enus/5_Scientific/c_time/time_server.html
struct tai_timestamp {
    std::uint64_t secs;
    std::uint32_t nanosecs;

    long double combined_secs() const;
};
std::ostream& operator<<(std::ostream& stream, const tai_timestamp& time);

/// Time measured in relation to the UTC system, stored with up to ~1 ns precision.
/// Here, UTC is Coordinated Universal Time - Civil time system as measured at
/// longitude zero. Kept adjusted to earth rotation by use of leap seconds. Also
/// known as Zulu Time. Replaced the similar system known as Greenwich Mean Time.
/// For more see http://aa.usno.navy.mil/faq/docs/UT.html
struct utc_timestamp {
    std::uint64_t secs; /// seconds since the UTC epoch (1970-01-01 00:00:00)
    std::uint32_t nanosecs; /// nanoseconds since the second

    long double combined_secs() const;

    /// Use the system time to sample a timestamp of the current time point.
    /// With NTP sync, this usually provides precision on the order of ~10 ms.
    static utc_timestamp now();

    /// Convenience conversion from boost universal time.
    static utc_timestamp from_universal_ptime(const boost::posix_time::ptime& time);

    /// Convenience conversion to boost universal time.
    boost::posix_time::ptime to_universal_ptime() const;
};
std::ostream& operator<<(std::ostream& stream, const utc_timestamp& time);