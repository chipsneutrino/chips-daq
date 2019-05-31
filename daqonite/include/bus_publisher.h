#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include "daq_handler.h"
#include <util/control_msg.h>

class BusPublisher {
public:
    using message_type = DaqoniteStateMessage;

    explicit BusPublisher(std::shared_ptr<DAQHandler> daq_handler);
    virtual ~BusPublisher() = default;

    void runAsync();
    void join();

private:
    std::atomic_bool running_;
    std::unique_ptr<std::thread> comm_thread_;
    std::unique_ptr<std::thread> status_thread_;

    std::list<message_type> publish_queue_;
    std::condition_variable cv_publish_queue_;
    std::mutex mtx_publish_queue_;
    std::chrono::milliseconds reconnect_interval_;

    void communicationThread();

    std::condition_variable cv_status_thread_;
    std::mutex mtx_status_thread_;
    std::chrono::milliseconds status_interval_;
    std::shared_ptr<DAQHandler> daq_handler_;

    void statusThread();

    void publishStatus();
};
