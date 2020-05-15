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
    , data_run_ {}
    , data_run_serialiser_ {}
    , io_service_ { new io_service }
    , run_work_ { new io_service::work(*io_service_) }
    , thread_group_ {}
    , data_handler_ { new DataHandler }
    , hit_receivers_ {}
    , scheduling_ { new SpillSchedulers }
{
    setUnitName("DAQHandler");
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
    const std::size_t n_threads { 8 };

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

    // TODO: get these from a config file
    clb_ports_ = { 57001, 57002, 57003, 57004, 57005, 57006, 57007, 57008 };
    bbb_ports_ = { 57101, 57102, 57103, 57104, 57105, 57106, 57107, 57108 };

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
    if (data_run_) {
        handleStopRunCommand();
    }

    // Set the mode to data taking
    data_run_ = std::make_shared<DataRun>(which, output_directory_path_, scheduling_);
    data_run_serialiser_ = std::make_shared<DataRunSerialiser>(data_run_);

    data_run_->start();
    log(INFO, "Started data run: {}", data_run_->logDescription());

    // Start a data_handler run
    data_handler_->startRun(data_run_, data_run_serialiser_);

    for (const auto& hit_receiver : hit_receivers_) {
        hit_receiver->startRun(data_run_);
    }
}

void DAQHandler::handleStopRunCommand()
{
    if (!data_run_) {
        return;
    }

    data_run_->stop();
    log(INFO, "Stopped data run: {}", data_run_->logDescription());

    for (const auto& hit_receiver : hit_receivers_) {
        hit_receiver->stopRun();
    }

    // Stop the data_handler run
    data_handler_->stopRun();

    // Stop the serialiser.
    data_run_serialiser_->notifyJoin();
    data_run_serialiser_->join();

    // clean up
    data_run_serialiser_.reset();
    data_run_.reset();
}

void DAQHandler::handleExitCommand()
{
    log(INFO, "Exit");
    handleStopRunCommand();
    run_work_.reset();
    io_service_->stop();
}
