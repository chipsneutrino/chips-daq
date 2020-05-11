/**
 * DAQHandler - Handler class for combining data streams
 */

#include <cstring>

#include <util/elastic_interface.h>

#include "bbb_hit_receiver.h"
#include "clb_hit_receiver.h"
#include "daq_handler.h"

DAQHandler::DAQHandler(bool collect_clb_data, bool collect_bbb_data, const std::string& data_path)
    : collect_clb_data_ { collect_clb_data }
    , collect_bbb_data_ { collect_bbb_data }
    , clb_ports_ {}
    , bbb_ports_ {}
    , n_threads_ {}
    , mode_ { false }
    , run_type_ {}
    , io_service_ { new boost::asio::io_service }
    , run_work_ { new boost::asio::io_service::work(*io_service_) }
    , thread_group_ {}
    , data_handler_ { new DataHandler(data_path) }
    , hit_receivers_ {}
{
    clb_ports_.push_back(57001);
    clb_ports_.push_back(57002);
    clb_ports_.push_back(57003);
    clb_ports_.push_back(57004);
    clb_ports_.push_back(57005);
    clb_ports_.push_back(57006);
    clb_ports_.push_back(57007);
    clb_ports_.push_back(57008);

    bbb_ports_.push_back(57101);
    bbb_ports_.push_back(57102);
    bbb_ports_.push_back(57103);
    bbb_ports_.push_back(57104);
    bbb_ports_.push_back(57105);
    bbb_ports_.push_back(57106);
    bbb_ports_.push_back(57107);
    bbb_ports_.push_back(57108);

    // Calculate thread count
    n_threads_ = 0;

    if (collect_clb_data_) {
        n_threads_ += clb_ports_.size();
    }

    if (collect_bbb_data_) {
        n_threads_ += bbb_ports_.size();
    }

    setupHandlers();
}

void DAQHandler::setupHandlers()
{
    // Setup the CLB handler (if required)
    for (const int port : clb_ports_) {
        hit_receivers_.emplace_back(new CLBHitReceiver(io_service_, data_handler_, &mode_, port, hit_receivers_.size())); // FIXME: mode_
    }

    // Setup the BBB handler (if required)
    for (const int port : bbb_ports_) {
        hit_receivers_.emplace_back(new BBBHitReceiver(io_service_, data_handler_, &mode_, port, hit_receivers_.size())); // FIXME: mode_
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

void DAQHandler::handleConfigCommand(std::string config_file)
{
    g_elastic.log(INFO, "DAQonite: Config");
}

void DAQHandler::handleStartDataCommand()
{
    g_elastic.log(INFO, "DAQonite: Starting Data");

    // Call the first work method to the optical data
    for (const auto& hit_receiver : hit_receivers_) {
        hit_receiver->workOpticalData();
    }
}

void DAQHandler::handleStopDataCommand()
{
    g_elastic.log(INFO, "DAQonite: Stopping Data");

    //TODO: Stop the work method for the optical data
}

void DAQHandler::handleStartRunCommand(RunType which)
{
    g_elastic.log(INFO, "DAQonite: Starting Run");
    // If we are currently running first stop the current run
    if (mode_ == true) {
        g_elastic.log(INFO, "DAQ Handler stopping current mine");
        handleStopRunCommand();
    }

    // Start a data_handler run
    data_handler_->startRun(which);

    // Set the mode to data taking
    run_type_ = which;
    mode_ = true;
}

void DAQHandler::handleStopRunCommand()
{
    g_elastic.log(INFO, "DAQonite: Stopping Run");
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
    g_elastic.log(INFO, "DAQControl: Exit");
    handleStopRunCommand();
    run_work_.reset();
    io_service_->stop();
}
