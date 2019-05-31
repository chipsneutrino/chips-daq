/**
 * DAQHandler - Handler class for combining data streams
 */

#include <cstring>

#include "daq_handler.h"
#include <util/elastic_interface.h>

DAQHandler::DAQHandler(bool collect_clb_data, bool collect_bbb_data)
    : collect_clb_data_{ collect_clb_data }
    , collect_bbb_data_{ collect_bbb_data }
    , clb_ports_{}
    , n_threads_{}
    , mode_{ false }
    , run_type_{}
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_group_{}
    , data_handler_{ new DataHandler }
    , clb_handlers_{}
    , bbb_handler_{}
{
    clb_ports_.push_back(56015); // TODO: use configurable CLB ports here

    // Calculate thread count
    n_threads_ = 0;

    if (collect_clb_data_) {
        n_threads_ += clb_ports_.size();
    }

    if (collect_bbb_data_) {
        n_threads_ += 1;
    }

    setupHandlers();
}

void DAQHandler::setupHandlers()
{
    // Setup the CLB handler (if required)
    if (collect_clb_data_) {
        for (const int port : clb_ports_) {
            clb_handlers_.emplace_back(new CLBHandler(io_service_, data_handler_, &mode_, port, clb_handlers_.size())); // FIXME: mode_
        }
    }

    // Setup the BBB handler (if required)
    if (collect_bbb_data_) {
        bbb_handler_ = std::unique_ptr<BBBHandler>{ new BBBHandler }; // TODO: std::make_unique in c++14
    }
}

void DAQHandler::run()
{
    g_elastic.log(INFO, "DAQ Handler started ({}) ({}{})", n_threads_, collect_clb_data_ ? "clb" : "", collect_bbb_data_ ? ",bbb" : "");

    // Setup the thread group and call io_service.run() in each
    g_elastic.log(INFO, "DAQ Handler starting I/O service on {} threads", n_threads_);
    for (int i = 0; i < n_threads_; ++i) {
        thread_group_.create_thread(boost::bind(&DAQHandler::ioServiceThread, this));
    }

    // Wait for all the threads to finish
    thread_group_.join_all();
    data_handler_->join();

    g_elastic.log(INFO, "DAQ Handler finished.");
}

void DAQHandler::ioServiceThread()
{
    io_service_->run();
}

void DAQHandler::handleStartCommand(RunType which)
{
    // If we are currently running first stop the current run
    if (mode_ == true) {
        g_elastic.log(INFO, "DAQ Handler stopping current mine");
        handleStopCommand();
    }

    // Start a data_handler run
    data_handler_->startRun(which);

    // Set the mode to data taking
    run_type_ = which;
    mode_ = true;

    // Call the first work method to the optical data
    for (const auto& clb_handler : clb_handlers_) {
        clb_handler->workOpticalData();
    }
}

void DAQHandler::handleStopCommand()
{
    // Check we are actually running
    if (mode_ == true) {
        // Set the mode to monitoring
        mode_ = false;

        // Stop the data_handler run
        data_handler_->stopRun();
    } else {
        g_elastic.log(INFO, "DAQ Handler already stopped mining");
    }
}

void DAQHandler::handleExitCommand()
{
    handleStopCommand();
    run_work_.reset();
    io_service_->stop();
}
