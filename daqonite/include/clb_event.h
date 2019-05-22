/**
 * CLB Event - Data structure for merge-sorting hits
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#ifndef CLB_EVENT_H
#define CLB_EVENT_H

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

struct CLBEvent {
    std::uint32_t PomId; ///< Header POM ID (4 bytes)
    std::uint8_t Channel; ///< Hit Channel ID (1 bytes)
    std::uint32_t Timestamp_s; ///< Header timestamp (4 bytes)
    std::uint32_t Timestamp_ns; ///< Hit timestamp (4 bytes)
    std::int8_t Tot; ///< Hit TOT value (1 bytes)

    double SortKey; ///< Compound timestamp used for sorting

    inline bool operator<(const CLBEvent& other) const noexcept
    {
        return SortKey < other.SortKey;
    }

    inline bool operator>(const CLBEvent& other) const noexcept
    {
        return SortKey > other.SortKey;
    }
};

class CLBEventQueue : public std::vector<CLBEvent> {
};

class CLBEventMultiQueue : public std::unordered_map<std::uint32_t, CLBEventQueue> {
public:
    std::mutex write_mutex {};

    inline CLBEventQueue& get_queue_for_writing(std::uint32_t pom_id)
    {
        auto it = find(pom_id);
        if (it == end()) {
            std::tie(it, std::ignore) = emplace(pom_id, CLBEventQueue {});
        }

        return it->second;
    }
};

#endif /* CLB_EVENT_H */