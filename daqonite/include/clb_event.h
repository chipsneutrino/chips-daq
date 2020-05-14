/**
 * CLB Event - Data structure for merge-sorting hits
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <util/timestamp.h>

struct hit {
    std::uint32_t plane_number {}; /// uniquely identifies the plane
    std::uint8_t channel_number {}; /// which PMT in the plane was hit
    tai_timestamp timestamp {}; /// when the hit occurred
    std::uint8_t tot {}; /// time over threshold, corresponds with energy
    std::uint8_t adc0 {}; /// value of the ADC, only for Madison planes

    long double sort_key {}; /// precalculated compound timestamp, useful for sorting

    inline bool operator<(const hit& other) const { return sort_key < other.sort_key; }
    inline bool operator>(const hit& other) const { return sort_key > other.sort_key; }

    static constexpr std::uint8_t NO_ADC0 { 0 }; // TODO: pick an uncommon ADC value
};

class HitQueue : public std::vector<hit> {
};

class HitMultiQueue : public std::unordered_map<std::uint32_t, HitQueue> {
public:
    std::mutex write_mutex {}; ///< Used by CLBHandlers and DataManager, serves to make sure no-one is writing to a closed queue.
    std::atomic_bool closed_for_writing { false }; ///< Is this queue not to be written into?

    inline HitQueue& get_queue_for_writing(std::uint32_t plane_number)
    {
        auto it = find(plane_number);
        if (it == end()) {
            std::tie(it, std::ignore) = emplace(plane_number, HitQueue {});
        }

        return it->second;
    }
};
