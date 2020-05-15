#pragma once

#include <vector>

#include <spill_scheduling/spill_data_slot.h>
#include <util/timestamp.h>

/**********************************************************************************************************************
 * DANGER:
 * 
 * To prevent data from being copied unnecessarily, spills are juggled around as raw pointers. Therefore, if not managed
 * properly they are prone to memory leaks. To prevent this from happening, their life and ownership cycle is clarified
 * explicitly below:
 * 
 *   1.  Spills created by SpillSchedulers (using the `new` operator). The SpillScheduler sets `start_time`, `end_time`.
 *       The `created` flag set implicitly by the constructor.
 * 
 *   2.  After creation, Spills are added to the current SpillSchedule by the SpillScheduler. Since the schedule is
 *       managed by SpillSchedule, this implies ownership transfer. The SpillSchedule also allocates data structures for all
 *       new Spills.
 * 
 *   3.  For the most of their lifetime Spills are owned by the SpillSchedule, which closes them when the run ends or when
 *       their end time expires during an ongoing run. Closed Spills are either deleted by the SpillSchedule straight away
 *       (if empty) or passed on to the DataRunSerialiser, constituting another ownership transfer.
 * 
 *   4.  The DataRunSerialiser acts as a sink and deletes all Spills it receives.
 * 
 **********************************************************************************************************************/

struct Spill {
    tai_timestamp start_time; ///< Start timestamp for events.
    tai_timestamp end_time; ///< End timestamp for events.

    std::size_t spill_number; ///< Sequential identifier (unique within the scope of a run) used for logging.
    bool created; ///< Was the spill just created by the scheduler and needs DS allocation?
    bool started; ///< Was the spill "touched" by any data taking thread?
    utc_timestamp last_updated_time; ///< Time of last "touch"

    SpillDataSlot* data_slots; ///< Multiple data slots, one for each hit receiver
    std::size_t n_data_slots; ///< Number of valid items in `data_slots`

    explicit Spill()
        : start_time {}
        , end_time {}
        , spill_number {}
        , created { true }
        , started {}
        , last_updated_time {}
        , data_slots {}
        , n_data_slots {}
    {
    }

    // no copy semantics
    Spill(const Spill& other) = delete;
    Spill& operator=(const Spill& other) = delete;

    ~Spill()
    {
        if (data_slots != nullptr) {
            delete[] data_slots;
            data_slots = nullptr;
        }
    }
};

using SpillPtr = Spill*;
