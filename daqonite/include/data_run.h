/**
 * DataRun - Aggregates all information that is related to a single CHIPS run.
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <chrono>
#include <string>

#include <util/control_msg.h>

#include "scheduling_pool.h"

enum class DataRunState : int {
    NotStarted,
    Running,
    Stopped
};

// TODO: move run type here

class DataRun {
public:
    explicit DataRun(RunType type, const std::string& output_directory_path, const std::shared_ptr<SchedulingPool>& scheduling_pool);

    void start();
    void stop();

    inline DataRunState getState() const { return state_; }
    inline RunType getType() const { return type_; }
    inline const std::string& getOutputFilePath() const { return output_file_path_; }
    std::shared_ptr<BatchScheduler> getScheduler() const;

    std::string logDescription() const;

private:
    // TODO: we would like this to be a TAI clock
    using clock = std::chrono::high_resolution_clock;

    DataRunState state_;
    clock::time_point pc_time_started_;
    clock::time_point pc_time_stopped_;

    static std::string formatTime(const clock::time_point& time);
    static std::string formatType(RunType type);

    std::shared_ptr<SchedulingPool> scheduling_;

    RunType type_;
    std::uint64_t number_;
    std::string output_directory_path_;
    std::string output_file_path_;

    void setOutputFileName();
};