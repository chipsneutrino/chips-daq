#include <functional>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/sub0.h>

#include <util/config.h>

#include "command_receiver.h"

CommandReceiver::CommandReceiver()
    : Logging {}
    , handler_ {}
    , url_ { g_config.lookupString("bus.control") }
    , running_ { false }
    , receiver_thread_ {}
    , cv_receiver_thread_ {}
    , mtx_receiver_thread_ {}
{
    setUnitName("CommandReceiver");
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
    receiver_thread_ = std::unique_ptr<std::thread> { new std::thread(std::bind(&CommandReceiver::receiverThread, this)) }; // TODO: std::make_unique in c++14
}

void CommandReceiver::join()
{
    if (!running_) {
        return;
    }

    running_ = false;
    cv_receiver_thread_.notify_one();

    if (receiver_thread_ && receiver_thread_->joinable()) {
        receiver_thread_->join();
    }

    receiver_thread_.reset();
}

void CommandReceiver::receiverThread()
{
    if (!handler_) {
        log(WARNING, "CommandReceiver started without a CommandHandler, commands will not be carried out! Terminating to avoid crash.");
        return;
    }

    log(INFO, "CommandReceiver started");

    while (running_) {
        try {
            auto sock = nng::sub::open();
            nng::sub::set_opt_subscribe(sock, "");
            nng::set_opt_recv_timeout(sock, 200);
            sock.dial(url_.c_str());

            ControlMessage message {};
            while (running_) {
                try {
                    sock.recv(nng::view { &message, sizeof(message) });
                } catch (const nng::exception& e) {
                    switch (e.get_error()) {
                    case nng::error::timedout:
                        continue;
                    default:
                        throw;
                    }
                }

                processMessage(message);
            }
        } catch (const nng::exception& e) {
            log(ERROR, "CommandReceiver caught error when listening: {}", e.what());
            log(INFO, "CommandReceiver will reconnect in 5 seconds");

            std::unique_lock<std::mutex> lk { mtx_receiver_thread_ };
            cv_receiver_thread_.wait_for(lk, std::chrono::seconds(5), [this] { return !running_; });
        }
    }

    log(INFO, "CommandReceiver finished");
}

void CommandReceiver::processMessage(const ControlMessage& message)
{
    switch (message.Discriminator) {
    case ControlMessage::Config::Discriminator:
        handler_->handleConfigCommand(message.Payload.pConfig.config_file);
        break;
    case ControlMessage::StartData::Discriminator:
        handler_->handleStartDataCommand();
        break;
    case ControlMessage::StopData::Discriminator:
        handler_->handleStopDataCommand();
        break;
    case ControlMessage::StartRun::Discriminator:
        handler_->handleStartRunCommand(message.Payload.pStartRun.run_type);
        break;
    case ControlMessage::StopRun::Discriminator:
        handler_->handleStopRunCommand();
        break;
    case ControlMessage::Exit::Discriminator:
        handler_->handleExitCommand();
        break;
    default:
        log(WARNING, "CommandReceiver got message with unknown discriminator: {}", message.Discriminator);
        break;
    }
}
