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
#include <util/timestamp.h>

#include "spill_schedulers.h"

enum class DataRunState : int {
    NotStarted,
    Running,
    Stopped
};

// TODO: move run type here

class DataRun {
public:
    explicit DataRun(RunType type, const std::string& output_directory_path, const std::shared_ptr<SpillSchedulers>& scheduling_pool);

    void start();
    void stop();

    inline DataRunState getState() const { return state_; }
    inline std::uint64_t getNumber() const { return number_; }
    inline RunType getType() const { return type_; }
    inline const utc_timestamp& getTimeStarted() const { return pc_time_started_; }
    inline const utc_timestamp& getTimeStopped() const { return pc_time_stopped_; }
    inline const std::string& getOutputFilePath() const { return output_file_path_; }
    std::shared_ptr<BasicSpillScheduler> getScheduler() const;

    std::string logDescription() const;

private:
    DataRunState state_;
    utc_timestamp pc_time_started_;
    utc_timestamp pc_time_stopped_;

    static std::string formatType(RunType type);

    std::shared_ptr<SpillSchedulers> scheduling_;

    RunType type_;
    std::uint64_t number_;
    std::string output_directory_path_;
    std::string output_file_path_;

    void setOutputFileName();
};