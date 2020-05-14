/**
 * PMTHitQueue and PMTMultiPlaneHitQueue - data structures to hold PMT hits
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <util/pmt_hit.h>

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
