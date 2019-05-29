#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "control_msg.h"

class CommandHandler {
public:
    virtual ~CommandHandler() = default;

    virtual void handleStartCommand(control_msg::daq::start_run::run_type which) = 0;
    virtual void handleStopCommand() = 0;
    virtual void handleExitCommand() = 0;
};

class CommandReceiver {
public:
    explicit CommandReceiver();
    virtual ~CommandReceiver() = default;

    void setHandler(std::shared_ptr<CommandHandler> handler);

    void runAsync();
    void join();

private:
    std::shared_ptr<CommandHandler> handler_;

    std::atomic_bool running_;
    std::unique_ptr<std::thread> receiver_thread_;

    void receiverThread();
};
