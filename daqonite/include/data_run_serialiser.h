#pragma once

#include <memory>

#include <boost/lockfree/queue.hpp>

#include <spill_scheduling/spill.h>
#include <util/async_component.h>
#include <util/logging.h>
#include <util/pmt_hit_queues.h>

#include "data_run.h"

class DataRunSerialiser : protected Logging, public AsyncComponent {
public:
    explicit DataRunSerialiser(const std::shared_ptr<DataRun>& data_run);
    virtual ~DataRunSerialiser() = default;

    void serialiseSpill(SpillPtr spill);

protected:
    void run() override;

private:
    using SpillQueue = boost::lockfree::queue<SpillPtr, boost::lockfree::capacity<16>>;
    SpillQueue waiting_spills_; ///< Thread-safe FIFO queue for closed spills pending merge-sort

    std::shared_ptr<DataRun> data_run_;

    /// Implementation of conventional insert-sort algorithm used to pre-sort CLB queues.
    static std::size_t insertSort(PMTHitQueue& queue) noexcept;
};