/**
 * TDUSpillScheduler - A scheduler that creates spills based on signals reported
 * by a NOvA TDU at Fermilab
 * 
 * Under the hood, the scheduler spins up an XML/RPC server that consumes the TDU
 * data. For selected signal types, spills are created with configurable time offsets.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <atomic>
#include <ctime>
#include <memory>
#include <thread>

#include <XmlRpc.h>

#include <spill_scheduling/basic_spill_scheduler.h>
#include <spill_scheduling/trigger_predictor.h>

/// Clock signal conversion from NoVA to UTC. Nicked from NSS time utils.
void convertNovaTimeToUnixTime(const std::uint64_t& inputNovaTime, struct timeval& outputUnixTime);

class TDUSpillScheduler : public BasicSpillScheduler {
    int port_; ///< Port where spill messages are expected to come.
    std::size_t trigger_memory_size_; ///< How many past triggers to remember (more = robust, less = quickly adapting)
    double init_period_guess_; ///< Initial guess of trigger period in seconds. Will be eventually overwritten.
    std::size_t n_batches_ahead_; ///< How many batches to open in the future?
    double time_window_radius_; ///< Duration around spill time for batches.
    bool bypass_os_time_; ///< If true, spill scheduler will wait for optical data to get timestamps instead of calling OS functions. Use if OS time is not in sync with WR time.

    std::atomic_bool spill_server_running_; ///< Is the spill server supposed to be running?
    std::unique_ptr<std::thread> spill_server_thread_; ///< Spill server thread.

    std::unique_ptr<XmlRpc::XmlRpcServer> spill_server_; ///< XML-RPC server handling spill requests.
    std::shared_ptr<TriggerPredictor> predictor_; ///< Spill interval predictor.

    /// Main loop of the spill server thread.
    void workSpillServer();

public:
    explicit TDUSpillScheduler(int port, std::size_t trigger_memory_size, double init_period_guess, std::size_t n_batches_ahead, double time_window_radius);
    void updateSchedule(SpillSchedule& schedule, const tai_timestamp& last_approx_timestamp) override;

    /// Wait until spill server terminates.
    void join();
};