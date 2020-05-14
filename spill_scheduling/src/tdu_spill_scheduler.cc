#include <functional>

#include "tdu_spill_scheduler.h"
#include "xml_rpc_spill_method.h"

TDUSpillScheduler::TDUSpillScheduler(int port, std::size_t trigger_memory_size,
    double init_period_guess, std::size_t n_batches_ahead, double time_window_radius)
    : BasicSpillScheduler {}
    , port_ { port }
    , trigger_memory_size_ { trigger_memory_size }
    , init_period_guess_ { init_period_guess }
    , n_batches_ahead_ { n_batches_ahead }
    , time_window_radius_ { time_window_radius }
    , spill_server_running_ {}
    , spill_server_thread_ {}
    , bypass_os_time_ { false } // TODO: this should be a configurable setting
{
    setUnitName("TDUSpillScheduler");

    spill_server_running_ = true;
    spill_server_thread_ = std::unique_ptr<std::thread> { new std::thread(std::bind(&TDUSpillScheduler::workSpillServer, this)) };
}

void TDUSpillScheduler::join()
{
    if (spill_server_thread_ && spill_server_thread_->joinable()) {
        spill_server_running_ = false;
        spill_server_thread_->join();
    }

    spill_server_thread_.reset();
}

void TDUSpillScheduler::updateSchedule(SpillSchedule& schedule, const tai_timestamp& last_approx_timestamp)
{
    /* FIXME: this entire function needs to be reimplemented */

    /*
    // TODO: configure these
    static const std::size_t n_batches_ahead_ = 8;
    static const double window_half_width_s_ = 0.005;

    if (schedule.size() >= n_batches_ahead_) {
        return;
    }

    // FIXME: race condition? get both at the same time
    const tai_timestamp last_timestamp = predictor_->lastTimestamp();
    const TimeDiff learned_interval = predictor_->learnedInterval();

    // TODO: set up intervals based on the predicted triggers
    // This can be done once:
    //   (1) we know what time representation we'll use
    //   (2) we know how trigger predictions and signals will be matched
    //   (3) white rabbits start giving sensible timestamps

    tai_timestamp schedule_basis_timestamp {};
    if (!bypass_os_time_) {
        // The basis timestamp comes from the OS.
        // Hope it's /roughly/ in sync with the data. Fortunately, we only need second-ish precision.
        schedule_basis_timestamp = tai_timestamp::now();
    } else {
        // Use the last approximate timestamp received in the optical stream.
        // If data just started coming in, this may still be zero and cause the temporary "no packets received" warning.
        schedule_basis_timestamp = last_approx_timestamp;
    }

    if (!schedule.empty()) {
        // Ensure that the timestamp we use as a basis for scheduling is as recent as possible.
        // This prevents creating batch in the past if the schedule capacity is too small.
        const tai_timestamp last_scheduled_timestamp { schedule.back().end_time };

        if (last_scheduled_timestamp >= schedule_basis_timestamp) {
            schedule_basis_timestamp = last_scheduled_timestamp;
        } else {
            log(WARNING, "Data ({}) is more recent than the last scheduled batch ({}). Some batches likely were missed. Try increasing schedule capacity!",
                schedule_basis_timestamp, last_scheduled_timestamp);
        }
    }

    if (schedule_basis_timestamp < 1e-3) {
        // If there is no data, wait for more.
        log(WARNING, "No packets received. Cannot schedule batches yet.");
        return;
    }

    // Determine at which spill the scheduling starts.
    int coef;
    if (schedule_basis_timestamp < last_timestamp) {
        // The last spill (and possibly some more before it) have not been scheduled.
        coef = 0;
    } else {
        // Extrapolate the next spill that hasn't been observed/scheduled.
        coef = 1 + (schedule_basis_timestamp - last_timestamp) / learned_interval;
    }

    log(INFO, "Will schedule {} more batches. Starting after timestamp {} (extrapolation factor {}x).",
        n_batches_ahead_ - schedule.size(), schedule_basis_timestamp, coef);

    while (schedule.size() < n_batches_ahead_) {
        const Timestamp center_timestamp = last_timestamp + learned_interval * coef;

        Batch next {};
        next.created = true;
        next.start_time = center_timestamp - window_half_width_s_;
        next.end_time = center_timestamp + window_half_width_s_;

        schedule.push_back(std::move(next));
        ++coef;
    }
    */
}

void TDUSpillScheduler::workSpillServer()
{
    using namespace XmlRpc;

    log(INFO, "Up and running at port {}!", port_);

    std::unique_ptr<XmlRpc::XmlRpcServer> spill_server { new XmlRpcServer };
    auto predictor { std::make_shared<TriggerPredictor>(trigger_memory_size_, init_period_guess_) };
    XMLRPCSpillMethod spill_method { spill_server.get(), predictor };
    XmlRpc::setVerbosity(0);

    spill_server->bindAndListen(port_);

    while (spill_server_running_) {
        spill_server->work(1);
    }

    spill_server->shutdown();

    log(INFO, "Signing off!");
}