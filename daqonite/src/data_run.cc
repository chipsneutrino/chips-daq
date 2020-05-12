#include <iomanip>
#include <sstream>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "data_run.h"

DataRun::DataRun()
    : state_ { DataRunState::NotStarted }
    , pc_time_started_ {}
    , pc_time_stopped_ {}
    , type_ {}
{
}

void DataRun::start(RunType type)
{
    type_ = type;
    pc_time_started_ = clock::now();
    state_ = DataRunState::Running;
}

void DataRun::stop()
{
    pc_time_stopped_ = clock::now();
    state_ = DataRunState::Stopped;
}

std::string DataRun::logDescription() const
{
    switch (state_) {
    case DataRunState::NotStarted:
        return fmt::format("[type = {}, state = not_started]", formatType(type_));
    case DataRunState::Running:
        return fmt::format("[type = {}, state = running, start_time = {}]",
            formatType(type_), formatTime(pc_time_started_));
    case DataRunState::Stopped:
        return fmt::format("[type = {}, state = stopped, start_time = {}, stop_time = {}]",
            formatType(type_), formatTime(pc_time_started_), formatTime(pc_time_stopped_));
    default:
        return "unknown";
    }
}

std::string DataRun::formatTime(const clock::time_point& time)
{
    // TODO: this needs to be rethought once the clock is final
    std::time_t converted { std::chrono::system_clock::to_time_t(time) };
    std::stringstream ss {};
    ss << std::put_time(std::localtime(&converted), "%F %T");
    return ss.str();
}

std::string DataRun::formatType(RunType type)
{
    switch (type) {
    case RunType::DataNormal:
        return "DataNormal";
    case RunType::Calibration:
        return "Calibration";
    case RunType::TestNormal:
        return "TestNormal";
    case RunType::TestFlasher:
        return "TestFlasher";
    default:
        return "Unknown";
    }
}
