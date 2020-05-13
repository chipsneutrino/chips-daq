#include <limits>

#include <fmt/format.h>

#include "timestamp.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

// Frequently used constants.
static const ptime UTC_EPOCH { date(1970, 1, 1) };
static const auto NS_PER_FRAC_S { static_cast<std::uint64_t>(time_duration(0, 0, 0, 1).total_nanoseconds()) };
static constexpr std::uint32_t NS_PER_S { 1000000000 };

// here we are assuming that fractional_second >= nanosecond
// TODO: make this ideally a static assert

tai_timestamp::tai_timestamp()
    : tai_timestamp { 0, 0 }
{
}

tai_timestamp::tai_timestamp(std::uint64_t s, std::uint32_t ns)
    : secs { s }
    , nanosecs { ns }
{
}

long double tai_timestamp::combined_secs() const
{
    return secs + 1e-9 * nanosecs;
}

bool tai_timestamp::empty() const
{
    return secs == 0 && nanosecs == 0;
}

void tai_timestamp::normalise()
{
    secs += nanosecs / NS_PER_S;
    nanosecs = nanosecs % NS_PER_S;
}

tai_timestamp tai_timestamp::min_time()
{
    tai_timestamp time {};
    time.secs = 0;
    time.nanosecs = 0;

    return time;
}

tai_timestamp tai_timestamp::max_time()
{
    tai_timestamp time {};
    time.secs = std::numeric_limits<decltype(time.secs)>::max();
    time.nanosecs = std::numeric_limits<decltype(time.nanosecs)>::max();

    return time;
}

std::ostream& operator<<(std::ostream& stream, const tai_timestamp& time)
{
    return stream << fmt::format("[TAI {:0.10f}]", time.combined_secs());
}

bool operator==(const tai_timestamp& lhs, const tai_timestamp& rhs)
{
    // Component-wise comparison.
    return lhs.secs == rhs.secs && lhs.nanosecs == rhs.nanosecs;
}

bool operator<(const tai_timestamp& lhs, const tai_timestamp& rhs)
{
    // Component-wise comparison.
    return lhs.secs == rhs.secs
        ? (lhs.nanosecs < rhs.nanosecs)
        : (lhs.secs < rhs.secs);
}

bool operator>(const tai_timestamp& lhs, const tai_timestamp& rhs)
{
    // Component-wise comparison.
    return lhs.secs == rhs.secs
        ? (lhs.nanosecs > rhs.nanosecs)
        : (lhs.secs > rhs.secs);
}

bool operator<=(const tai_timestamp& lhs, const tai_timestamp& rhs)
{
    // Delegate to operators.
    return lhs < rhs || lhs == rhs;
}

bool operator>=(const tai_timestamp& lhs, const tai_timestamp& rhs)
{
    // Delegate to operators.
    return lhs > rhs || lhs == rhs;
}

long double utc_timestamp::combined_secs() const
{
    return secs + 1e-9 * nanosecs;
}

std::ostream& operator<<(std::ostream& stream, const utc_timestamp& time)
{
    const auto boost_time { time.to_universal_ptime() };
    return stream << fmt::format("[UTC {}]", to_simple_string(boost_time));
}

bool operator==(const utc_timestamp& lhs, const utc_timestamp& rhs)
{
    // Component-wise comparison.
    return lhs.secs == rhs.secs && lhs.nanosecs == rhs.nanosecs;
}

bool operator<(const utc_timestamp& lhs, const utc_timestamp& rhs)
{
    // Component-wise comparison.
    return lhs.secs == rhs.secs
        ? (lhs.nanosecs < rhs.nanosecs)
        : (lhs.secs < rhs.secs);
}

bool operator>(const utc_timestamp& lhs, const utc_timestamp& rhs)
{
    // Component-wise comparison.
    return lhs.secs == rhs.secs
        ? (lhs.nanosecs > rhs.nanosecs)
        : (lhs.secs > rhs.secs);
}

bool operator<=(const utc_timestamp& lhs, const utc_timestamp& rhs)
{
    // Delegate to operators.
    return lhs < rhs || lhs == rhs;
}

bool operator>=(const utc_timestamp& lhs, const utc_timestamp& rhs)
{
    // Delegate to operators.
    return lhs > rhs || lhs == rhs;
}

utc_timestamp utc_timestamp::now()
{
    ptime t { microsec_clock::universal_time() };
    return from_universal_ptime(t);
}

utc_timestamp utc_timestamp::from_universal_ptime(const ptime& time)
{
    const time_duration time_since_epoch { time - UTC_EPOCH };

    utc_timestamp constructed_time {};
    constructed_time.secs = time_since_epoch.total_seconds();
    constructed_time.nanosecs = NS_PER_FRAC_S * time_since_epoch.fractional_seconds();
    return constructed_time;
}

boost::posix_time::ptime utc_timestamp::to_universal_ptime() const
{
    // Since boost does not have BOOST_DATE_TIME_HAS_NANOSECONDS by default...
    using ns = boost::date_time::subsecond_duration<time_duration, NS_PER_S>;
    return UTC_EPOCH + seconds(secs) + ns(nanosecs);
}
