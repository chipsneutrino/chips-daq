/**
 * BatchScheduler - Class responsible for scheduling data taking windows
 * 
 * These classes maintain batch schedule -- a series of time intervals, to
 * which taken data is matched. 
 *
 * Author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#ifndef BATCH_SCHEDULER_H_
#define BATCH_SCHEDULER_H_

#include <list>
#include <memory>
#include <chrono>

#include "clb_event.h"

struct Batch {
    double start_time;
    double end_time;
    
    bool started;
    std::chrono::steady_clock::time_point last_updated_time;

    CLBEventMultiQueue* clb_opt_data;

};

using BatchSchedule = std::list<Batch>;

/// Base class for all schedulers, needs to be inherited and implemented.
class BatchScheduler {
public:
    virtual ~BatchScheduler() = default;
    virtual void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) = 0;

};

/// Scheduler which produces only one infinite batch.
class InfiniteScheduler: public BatchScheduler {
public:
    void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) override;

};

/// Scheduler which produces batches of uniform length.
class RegularScheduler: public BatchScheduler {
    std::size_t n_batches_ahead_;
    double batch_duration_s_;

public:
    explicit RegularScheduler(std::size_t n_batches_ahead, std::chrono::milliseconds batch_duration);
    void updateSchedule(BatchSchedule& schedule, std::uint32_t last_approx_timestamp) override;

};

// TODO: SpillScheduler

#endif /* BATCH_SCHEDULER_H_ */