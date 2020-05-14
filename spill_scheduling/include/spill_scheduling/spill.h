#pragma once

#include <util/pmt_hit_queues.h>
#include <util/timestamp.h>

struct Spill {
    tai_timestamp start_time {}; ///< Start timestamp for events.
    tai_timestamp end_time {}; ///< End timestamp for events.

    int idx {}; ///< Batch index used for logging.
    bool created {}; ///< Was the batch just created by the scheduler and needs DS allocation?
    bool started {}; ///< Was the batch "touched" by any data taking thread?
    utc_timestamp last_updated_time {}; ///< Time of last "touch"

    PMTMultiPlaneHitQueue* clb_opt_data {}; ///< Data queues, one for each slot. Managed by DataHandler.
};
