#include <functional>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/sub0.h>

#include "command_receiver.h"
#include "elastic_interface.h"

CommandReceiver::CommandReceiver()
    : handler_{}
    , running_{ false }
    , receiver_thread_{}
{
}

void CommandReceiver::setHandler(std::shared_ptr<CommandHandler> handler)
{
    handler_ = std::move(handler);
}

void CommandReceiver::runAsync()
{
    if (running_) {
        return;
    }

    running_ = true;
    receiver_thread_ = std::unique_ptr<std::thread>{ new std::thread(std::bind(&CommandReceiver::receiverThread, this)) }; // TODO: std::make_unique in c++14
}

void CommandReceiver::join()
{
    if (!running_) {
        return;
    }

    running_ = false;

    if (receiver_thread_ && receiver_thread_->joinable()) {
        receiver_thread_->join();
    }

    receiver_thread_.reset();
}

void CommandReceiver::receiverThread()
{
    if (!handler_) {
        g_elastic.log(WARNING, "CommandReceiver started without a CommandHandler, commands will not be carried out! Terminating to avoid crash.");
        return;
    }

    g_elastic.log(INFO, "CommandReceiver started");

    while (running_) {
        try {
            auto sock = nng::sub::open();
            sock.set_opt_string(NNG_OPT_SUB_SUBSCRIBE, "");
            sock.dial(control_msg::daq::url);

            // FIXME: listen here
        } catch (const nng::exception& e) {
            g_elastic.log(ERROR, "CommandReceiver caught error when listening: {}", e.what());
            g_elastic.log(INFO, "CommandReceiver reconnecting in 5 seconds");
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    g_elastic.log(INFO, "CommandReceiver finished");
}
