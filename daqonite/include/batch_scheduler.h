/**
 * BatchScheduler - Class responsible for scheduling data taking windows
 * 
 * These classes maintain batch schedule -- a series of time intervals, to
 * which taken data is matched. 
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#pragma once

#include <chrono>
#include <list>

#include "clb_event.h"

struct Batch {
    double start_time; ///< Start timestamp for events.
    double end_time; ///< End timestamp for events.

    int idx; ///< Batch index used for logging.
    bool created; ///< Was the batch just created by the scheduler and needs DS allocation?
    bool started; ///< Was the batch "touched" by any data taking thread?
    std::chrono::steady_clock::time_point last_updated_time; ///< Time of last "touch"

    CLBEventMultiQueue* clb_opt_data; ///< Data queues, one for each slot. Managed by DataHandler.
};

using BatchSchedule = std::list<Batch>;

/// Base class for all schedulers, needs to be inherited and implemented.
class BatchScheduler {
public:
    virtual ~BatchScheduler() = default;
    virtual void beginScheduling() {}
    virtual void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) = 0;
    virtual void endScheduling() {}
};

/// Scheduler which produces only one infinite batch.
class InfiniteScheduler : public BatchScheduler {
public:
    void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) override;
};

/// Scheduler which produces batches of uniform length.
class RegularScheduler : public BatchScheduler {
    std::size_t n_batches_ahead_;
    double batch_duration_s_;

public:
    explicit RegularScheduler(std::size_t n_batches_ahead, std::chrono::milliseconds batch_duration);
    void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) override;
};
