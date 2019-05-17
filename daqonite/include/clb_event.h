/**
 * CLB Event - Data structure for merge-sorting hits
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#ifndef CLB_EVENT_H
#define CLB_EVENT_H

#include <cstdint>

struct CLBEvent {
    std::uint32_t  	PomId;			///< Header POM ID (4 bytes)
    std::uint8_t 	Channel;		///< Hit Channel ID (1 bytes)
    std::uint32_t 	Timestamp_s;	///< Header timestamp (4 bytes)
    std::uint32_t 	Timestamp_ns;	///< Hit timestamp (4 bytes)
    std::int8_t 	Tot;			///< Hit TOT value (1 bytes)

    double SortKey;                 ///< Compound timestamp used for sorting

    inline bool operator<(const CLBEvent& other) const noexcept
    {
        return SortKey < other.SortKey;
    }

};

#endif /* CLB_EVENT_H */