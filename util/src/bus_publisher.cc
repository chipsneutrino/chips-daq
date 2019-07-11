#include <functional>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>

#include "util/bus_publisher.h"
#include <util/elastic_interface.h>

template <typename T>
BusPublisher<T>::BusPublisher()
    : running_{ false }
    , comm_thread_{}
    , status_thread_{}
    , publish_queue_{}
    , cv_publish_queue_{}
    , mtx_publish_queue_{}
    , reconnect_interval_{ 5000 }
    , cv_status_thread_{}
    , mtx_status_thread_{}
    , status_interval_{ 200 }
{
}

template <typename T>
void BusPublisher<T>::runAsync()
{
    if (running_) {
        return;
    }

    running_ = true;
    comm_thread_ = std::unique_ptr<std::thread>{ new std::thread(std::bind(&BusPublisher::communicationThread, this)) };
    status_thread_ = std::unique_ptr<std::thread>{ new std::thread(std::bind(&BusPublisher::statusThread, this)) };
}

template <typename T>
void BusPublisher<T>::join()
{
    if (!running_) {
        return;
    }

    running_ = false;
    cv_publish_queue_.notify_one();
    cv_status_thread_.notify_one();

    if (status_thread_ && status_thread_->joinable()) {
        status_thread_->join();
    }

    if (comm_thread_ && comm_thread_->joinable()) {
        comm_thread_->join();
    }

    status_thread_.reset();
    comm_thread_.reset();
}

template <typename T>
void BusPublisher<T>::statusThread()
{
    g_elastic.log(INFO, "BusPublisher status thread started");

    while (running_) {
        publishStatus();

        std::unique_lock<std::mutex> lk{ mtx_status_thread_ };
        cv_status_thread_.wait_for(lk, status_interval_, [this] { return !running_; });
    }

    g_elastic.log(INFO, "BusPublisher status thread finished");
}

template <typename T>
void BusPublisher<T>::communicationThread()
{
    g_elastic.log(INFO, "BusPublisher communication thread started");

    while (running_) {
        try {
            auto sock = nng::pub::open();
            sock.listen(message_type::URL);
            g_elastic.log(INFO, "BusPublisher publishing to '{}'", message_type::URL);

            for (;;) {
                std::unique_lock<std::mutex> lk{ mtx_publish_queue_ };

                while (!publish_queue_.empty()) {
                    sock.send(nng::view{ &publish_queue_.front(), sizeof(message_type) });
                    publish_queue_.pop_front();
                }

                if (!running_) {
                    break;
                } else {
                    cv_publish_queue_.wait(lk, [this] { return !running_ || !publish_queue_.empty(); });
                }
            }
        } catch (const nng::exception& e) {
            g_elastic.log(ERROR, "BusPublisher caught error: {}: {}", e.who(), e.what());
            std::this_thread::sleep_for(reconnect_interval_);
        }
    }

    g_elastic.log(INFO, "BusPublisher communication thread finished");
}

template class BusPublisher<DaqoniteStateMessage>;
template class BusPublisher<DaqontrolStateMessage>;
template class BusPublisher<DaqsitterStateMessage>;
