/**
 * DAQHandler - Handler class for combining data streams
 */

#include <cstring>

#include <util/elastic_interface.h>

#include "bbb_hit_receiver.h"
#include "clb_hit_receiver.h"
#include "daq_handler.h"

DAQHandler::DAQHandler(const std::string& data_path)
    : Logging {}
    , clb_ports_ {}
    , bbb_ports_ {}
    , output_directory_path_ { data_path }
    , run_ {}
    , io_service_ { new io_service }
    , run_work_ { new io_service::work(*io_service_) }
    , thread_group_ {}
    , data_handler_ { new DataHandler }
    , hit_receivers_ {}
{
    setUnitName("DAQHandler");

    // TODO: get this from a config file
    clb_ports_.push_back(57001);
    clb_ports_.push_back(57002);
    clb_ports_.push_back(57003);
    clb_ports_.push_back(57004);
    clb_ports_.push_back(57005);
    clb_ports_.push_back(57006);
    clb_ports_.push_back(57007);
    clb_ports_.push_back(57008);

    // TODO: get this from a config file
    bbb_ports_.push_back(57101);
    bbb_ports_.push_back(57102);
    bbb_ports_.push_back(57103);
    bbb_ports_.push_back(57104);
    bbb_ports_.push_back(57105);
    bbb_ports_.push_back(57106);
    bbb_ports_.push_back(57107);
    bbb_ports_.push_back(57108);
}

void DAQHandler::createHitReceivers()
{
    // Get rid of any previous receivers.
    hit_receivers_.clear();

    // Setup the CLB handler (if required)
    for (const int port : clb_ports_) {
        hit_receivers_.emplace_back(new CLBHitReceiver(io_service_, data_handler_, port));
    }

    // Setup the BBB handler (if required)
    for (const int port : bbb_ports_) {
        hit_receivers_.emplace_back(new BBBHitReceiver(io_service_, data_handler_, port));
    }
}

void DAQHandler::run()
{
    // TODO: get this from a configuration file
    const std::size_t n_threads { clb_ports_.size() + bbb_ports_.size() };

    // Setup the thread group and call io_service.run() in each
    log(INFO, "Starting I/O service on {} threads", n_threads);
    for (std::size_t i = 0; i < n_threads; ++i) {
        thread_group_.create_thread([this] { io_service_->run(); });
    }

    // Wait for all the threads to finish
    thread_group_.join_all();
    data_handler_->join();

    log(INFO, "I/O service signing off.");
}

void DAQHandler::handleConfigCommand(std::string config_file)
{
    log(INFO, "Config");
    createHitReceivers();
}

void DAQHandler::handleStartDataCommand()
{
    log(INFO, "Starting Data");

    // Call the first work method to the optical data
    for (const auto& hit_receiver : hit_receivers_) {
        hit_receiver->startData();
    }
}

void DAQHandler::handleStopDataCommand()
{
    log(INFO, "Stopping Data");

    //TODO: Stop the work method for the optical data
    for (const auto& hit_receiver : hit_receivers_) {
        hit_receiver->stopData();
    }
}

void DAQHandler::handleStartRunCommand(RunType which)
{
    // If we are currently running first stop the current run
    if (run_) {
        handleStopRunCommand();
    }

    // Set the mode to data taking
    run_ = std::make_shared<DataRun>(which, output_directory_path_);
    run_->start();

    log(INFO, "Started data run: {}", run_->logDescription());

    // Start a data_handler run
    data_handler_->startRun(run_);

    for (const auto& hit_receiver : hit_receivers_) {
        hit_receiver->startRun(run_);
    }
}

void DAQHandler::handleStopRunCommand()
{
    if (!run_) {
        return;
    }

    run_->stop();
    log(INFO, "Stopped data run: {}", run_->logDescription());

    for (const auto& hit_receiver : hit_receivers_) {
        hit_receiver->stopRun();
    }

    // Stop the data_handler run
    data_handler_->stopRun();

    run_.reset();
}

void DAQHandler::handleExitCommand()
{
    log(INFO, "Exit");
    handleStopRunCommand();
    run_work_.reset();
    io_service_->stop();
}
