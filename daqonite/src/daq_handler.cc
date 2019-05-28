/**
 * DAQHandler - Handler class for combining data streams
 */

#include <cstring>

#include "daq_handler.h"
#include "elastic_interface.h"

DAQHandler::DAQHandler(bool collect_clb_data, bool collect_bbb_data)
    : collect_clb_data_{ collect_clb_data }
    , collect_bbb_data_{ collect_bbb_data }
    , clb_ports_{}
    , n_threads_{}
    , mode_{ false }
    , io_service_{ new boost::asio::io_service }
    , thread_group_{}
    , signal_set_{ *io_service_, SIGINT }
    , local_socket_{ *io_service_, udp::endpoint(udp::v4(), 1096) }
    , buffer_local_{}
    , data_handler_{ new DataHandler }
    , clb_handlers_{}
    , bbb_handler_{}
{
    clb_ports_.push_back(56015); // TODO: use configurable CLB ports here

    // Calculate thread count
    n_threads_ = 1; // one for the local socket

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

    // Setup and work the local socket (daqommand)
    udp::socket::receive_buffer_size option_local{ 33554432 };
    local_socket_.set_option(option_local);
    workLocalSocket();

    // Work the linux signals
    workSignals();

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

void DAQHandler::handleSignals(boost::system::error_code const& error, int signum)
{
    if (error) {
        // TODO: handle it separately!
        g_elastic.log(ERROR, "Signal handler caught error {}: {}", error.value(), error.category().name());
        workSignals();
        return;
    }

    switch (signum) {
    case SIGINT:
        [[gnu::fallthrough]];
    case SIGTERM:
        // This is just for nice output.
        std::cout << std::endl;

        g_elastic.log(INFO, "Received signal {}. Terminating...", signum);
        cmdExit();
        break;

    default:
        g_elastic.log(WARNING, "Signal handler caught unhandled signal {}.", signum);
        break;
    }

    // Incase we want to add other signals, you need to call the work method again
    workSignals();
}

void DAQHandler::workSignals()
{
    using namespace boost::asio::placeholders;
    signal_set_.async_wait(boost::bind(&DAQHandler::handleSignals, this, error, signal_number));
}

void DAQHandler::handleLocalSocket(boost::system::error_code const& error, std::size_t size)
{
    if (error) {
        g_elastic.log(ERROR, "DAQ Handler caught error {}: {}", error.value(), error.category().name());
        g_elastic.log(WARNING, "DAQ Handler stopping work on local socket due to error.");
        return;
    }

    bool continue_listening = true;

    if (std::strncmp(buffer_local_, "start", 5) == 0) {
        g_elastic.log(INFO, "DAQ Handler received start command");

        const int run_type = (int)buffer_local_[5] - 48; // TODO: this must be changed!
        if (run_type >= 0 || run_type < 5) {
            cmdStart(run_type);
        } else {
            g_elastic.log(WARNING, "Received invalid run type: {} (expected 0-4)", run_type);
        }
    } else if (std::strncmp(buffer_local_, "stop", 4) == 0) {
        g_elastic.log(INFO, "DAQ Handler received stop command");
        cmdStop();
    } else if (std::strncmp(buffer_local_, "exit", 4) == 0) {
        g_elastic.log(INFO, "DAQ Handler received exit command");
        cmdExit();
        continue_listening = false;
    } else {
        g_elastic.log(WARNING, "DAQ Handler received unknown command");
    }

    if (continue_listening) {
        workLocalSocket();
    }
}

void DAQHandler::cmdStart(int run_type)
{
    // If we are currently running first stop the current run
    if (mode_ == true) {
        g_elastic.log(INFO, "DAQ Handler stopping current mine");

        // Set the mode to monitoring
        mode_ = false;

        // Stop the data_handler run
        data_handler_->stopRun();
    }

    // Start a data_handler run
    data_handler_->startRun(run_type);

    // Set the mode to data taking
    mode_ = true;

    // Call the first work method to the optical data
    for (const auto& clb_handler : clb_handlers_) {
        clb_handler->workOpticalData();
    }
}

void DAQHandler::cmdStop()
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

void DAQHandler::cmdExit()
{
    cmdStop();
    io_service_->stop();
}

void DAQHandler::workLocalSocket()
{
    using namespace boost::asio::placeholders;
    local_socket_.async_receive(boost::asio::buffer(&buffer_local_[0], buffer_size_local),
        boost::bind(&DAQHandler::handleLocalSocket, this, error, bytes_transferred));
}
