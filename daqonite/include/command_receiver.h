#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include <util/control_msg.h>

class CommandHandler {
public:
    virtual ~CommandHandler() = default;

    virtual void handleStartCommand(RunType which) = 0;
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
    std::condition_variable cv_receiver_thread_;
    std::mutex mtx_receiver_thread_;

    void receiverThread();
    void processMessage(const ControlMessage& message);
};
