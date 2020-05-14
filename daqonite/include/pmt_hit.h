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

/// A hit of a single PMT
struct PMTHit {
    std::uint32_t plane_number {}; /// uniquely identifies the plane
    std::uint8_t channel_number {}; /// which PMT in the plane was hit
    tai_timestamp timestamp {}; /// when the hit occurred
    std::uint8_t tot {}; /// time over threshold, corresponds with energy
    std::uint8_t adc0 {}; /// value of the ADC, only for Madison planes

    long double sort_key {}; /// precalculated compound timestamp, useful for sorting

    inline bool operator<(const PMTHit& other) const { return sort_key < other.sort_key; }
    inline bool operator>(const PMTHit& other) const { return sort_key > other.sort_key; }

    static constexpr std::uint8_t NO_ADC0 { 0 }; // TODO: pick an uncommon ADC value
};

/// A sequence of hits that come from a single plane.
class PMTHitQueue : public std::vector<PMTHit> {
};

/// A collection of multiple hit queues, each corresponding to an individual
/// plane, indexed by plane number.
class PMTMultiPlaneHitQueue : public std::unordered_map<std::uint32_t, PMTHitQueue> {
public:
    std::mutex mutex {}; /// threads need to hold this mutex before accessing the entire queue
    // TODO: hit receivers should not compete for this mutex

    std::atomic_bool closed_for_writing { false }; /// if true, writing is no longer enabled

    inline PMTHitQueue& get_queue_for_writing(std::uint32_t plane_number)
    {
        auto it = find(plane_number);
        if (it == end()) {
            // this is the first time we see this plane number, create a new queue for it
            std::tie(it, std::ignore) = emplace(plane_number, PMTHitQueue {});
        }

        return it->second;
    }
};
