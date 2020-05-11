#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include <util/control_msg.h>
#include <util/logging.h>

template <class T>
class BusPublisher : protected Logging {
public:
    using message_type = T;

    explicit BusPublisher(const std::string& url);
    virtual ~BusPublisher() = default;

    void runAsync();
    void join();

protected:
    std::atomic_bool running_;
    std::string url_;
    std::unique_ptr<std::thread> comm_thread_;
    std::unique_ptr<std::thread> status_thread_;

    std::list<T> publish_queue_;
    std::condition_variable cv_publish_queue_;
    std::mutex mtx_publish_queue_;
    std::chrono::milliseconds reconnect_interval_;

    void communicationThread();

    std::condition_variable cv_status_thread_;
    std::mutex mtx_status_thread_;
    std::chrono::milliseconds status_interval_;

    void statusThread();

    virtual void publishStatus() = 0;
};
