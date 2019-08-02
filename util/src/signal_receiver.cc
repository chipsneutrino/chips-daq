#include <functional>

#include "signal_receiver.h"
#include <util/elastic_interface.h>

SignalReceiver::SignalReceiver()
    : handler_{}
    , running_{ false }
    , receiver_thread_{}
    , io_service_{}
    , signal_set_{ io_service_, SIGINT }
{
}

void SignalReceiver::setHandler(std::shared_ptr<CommandHandler> handler)
{
    handler_ = std::move(handler);
}

void SignalReceiver::runAsync()
{
    if (running_) {
        return;
    }

    running_ = true;
    receiver_thread_ = std::unique_ptr<std::thread>{ new std::thread(std::bind(&SignalReceiver::receiverThread, this)) }; // TODO: std::make_unique in c++14

    workSignals();
}

void SignalReceiver::join()
{
    if (!running_) {
        return;
    }

    running_ = false;
    io_service_.stop();

    if (receiver_thread_ && receiver_thread_->joinable()) {
        receiver_thread_->join();
    }

    receiver_thread_.reset();
}

void SignalReceiver::receiverThread()
{
    if (!handler_) {
        g_elastic.log(WARNING, "SignalReceiver started without a CommandHandler, commands will not be carried out! Terminating to avoid crash.");
        return;
    }

    g_elastic.log(INFO, "SignalReceiver started");
    io_service_.run();
    g_elastic.log(INFO, "SignalReceiver finished");
}

void SignalReceiver::handleSignals(boost::system::error_code const& error, int signum)
{
    if (error) {
        // TODO: handle it separately!
        g_elastic.log(ERROR, "Signal handler caught error {}: {}", error.value(), error.category().name());
        workSignals();
        return;
    }

    switch (signum) {
    case SIGINT:
        // This is just for nice output.
        std::cout << std::endl;

        g_elastic.log(INFO, "Received signal {}. Terminating...", signum);
        handler_->handleExitCommand();
        io_service_.stop();
        break;

    default:
        g_elastic.log(WARNING, "Signal handler caught unhandled signal {}.", signum);
        break;
    }

    // In case we want to add other signals, you need to call the work method again
    workSignals();
}

void SignalReceiver::workSignals()
{
    using namespace boost::asio::placeholders;
    signal_set_.async_wait(boost::bind(&SignalReceiver::handleSignals, this, error, signal_number));
}
