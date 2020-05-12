#include <fstream>
#include <iomanip>
#include <sstream>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <util/elastic_interface.h>

#include "data_run.h"

DataRun::DataRun(RunType type, const std::string& output_directory_path, const std::shared_ptr<SchedulingPool>& scheduling_pool)
    : state_ { DataRunState::NotStarted }
    , pc_time_started_ {}
    , pc_time_stopped_ {}
    , scheduling_ { scheduling_pool }
    , type_ { type }
    , number_ {}
    , output_directory_path_ { output_directory_path }
    , output_file_path_ {}
{
    setOutputFileName();
}

void DataRun::start()
{
    // Set the Elasticsearch ingest pipeline run.num and run.type
    g_elastic.run(number_, static_cast<int>(type_));

    pc_time_started_ = clock::now();
    state_ = DataRunState::Running;
}

void DataRun::stop()
{
    pc_time_stopped_ = clock::now();
    state_ = DataRunState::Stopped;

    // Set the Elasticsearch ingest pipeline run.num and run.type
    // Put as -1 for both when outside a run
    g_elastic.run(-1, -1);
}

std::string DataRun::logDescription() const
{
    switch (state_) {
    case DataRunState::NotStarted:
        return fmt::format("[type = {}, number = {}, state = not_started]",
            formatType(type_), number_);
    case DataRunState::Running:
        return fmt::format("[type = {}, number = {}, state = running, start_time = {}]",
            formatType(type_), number_, formatTime(pc_time_started_));
    case DataRunState::Stopped:
        return fmt::format("[type = {}, number = {}, state = stopped, start_time = {}, stop_time = {}]",
            formatType(type_), number_, formatTime(pc_time_started_), formatTime(pc_time_stopped_));
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

void DataRun::setOutputFileName()
{
    // TODO: this entire function needs to be streamlined

    static constexpr auto run_file_name { "runNumbers.dat" }; // TODO: make this a configurable setting
    const std::string run_file_path { fmt::format("{}/{}", output_directory_path_, run_file_name) }; // TODO: use a path join
    const auto run_type_no { static_cast<int>(type_) };

    std::uint64_t runNums[NUMRUNTYPES];
    std::ifstream runNumFile(run_file_path);
    if (runNumFile.fail()) {
        runNumFile.close();
        // The file does not yet exist so lets create it
        std::ofstream newFile(run_file_path);
        if (newFile.is_open()) {
            for (int i = 0; i < NUMRUNTYPES; i++) {
                if (run_type_no == i) {
                    newFile << 2 << "\n";
                } else {
                    newFile << 1 << "\n";
                }
            }
            newFile.close();
        } else {
            throw std::runtime_error("Unable to create runNumbers.dat!");
        }
    } else {
        // The file exists so read from it
        for (int i = 0; i < NUMRUNTYPES; i++) {
            runNumFile >> runNums[i];
            if (runNums[i] < 1) {
                runNums[i] = 1;
            }
            if (run_type_no == i + 1) {
                number_ = runNums[i];
            }
        }
        runNumFile.close();

        // Now create the updated file
        std::ofstream updateFile(run_file_path);
        if (updateFile.is_open()) {
            for (int i = 0; i < NUMRUNTYPES; i++) {
                if (run_type_no == i + 1) {
                    updateFile << runNums[i] + 1 << "\n";
                } else {
                    updateFile << runNums[i] << "\n";
                }
            }
            updateFile.close();
        } else {
            throw std::runtime_error("Unable to update runNumbers.dat!");
        }
    }

    // TODO: use filesystem path join here
    output_file_path_ = fmt::format("{}/type{}_run{:05d}.root", output_directory_path_, run_type_no, number_);
}

std::shared_ptr<BatchScheduler> DataRun::getScheduler() const
{
    // TODO: determine schedulers properly

    switch (type_) {
    case RunType::DataNormal:
        return scheduling_->spillScheduler();
    case RunType::Calibration:
        return scheduling_->regularScheduler();
    case RunType::TestNormal:
        return scheduling_->infiniteScheduler();
    case RunType::TestFlasher:
        return scheduling_->infiniteScheduler();
    }
}
