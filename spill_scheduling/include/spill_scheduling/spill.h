#pragma once

#include <vector>

#include <util/pmt_hit_queues.h>
#include <util/timestamp.h>

struct Spill {
    tai_timestamp start_time; ///< Start timestamp for events.
    tai_timestamp end_time; ///< End timestamp for events.

    int idx; ///< Batch index used for logging.
    bool created; ///< Was the batch just created by the scheduler and needs DS allocation?
    bool started; ///< Was the batch "touched" by any data taking thread?
    utc_timestamp last_updated_time; ///< Time of last "touch"

    PMTMultiPlaneHitQueue* opt_hit_queues; ///< Data queues, one for each slot. Managed by DataHandler.
    std::size_t n_data_slots;

    explicit Spill()
        : start_time {}
        , end_time {}
        , idx {}
        , created { true }
        , started {}
        , last_updated_time {}
        , opt_hit_queues {}
        , n_data_slots {}
    {
    }

    // no copy semantics
    Spill(const Spill& other) = delete;
    Spill& operator=(const Spill& other) = delete;

    ~Spill()
    {
        if (opt_hit_queues != nullptr) {
            delete[] opt_hit_queues;
            opt_hit_queues = nullptr;
        }
    }
};

using SpillPtr = Spill*;
