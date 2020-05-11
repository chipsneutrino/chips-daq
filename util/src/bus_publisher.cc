#include <functional>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>

#include "util/bus_publisher.h"

template <typename T>
BusPublisher<T>::BusPublisher(const std::string& url)
    : Logging {}
    , running_ { false }
    , url_ { url }
    , comm_thread_ {}
    , status_thread_ {}
    , publish_queue_ {}
    , cv_publish_queue_ {}
    , mtx_publish_queue_ {}
    , reconnect_interval_ { 5000 }
    , cv_status_thread_ {}
    , mtx_status_thread_ {}
    , status_interval_ { 200 }
{
    setUnitName("BusPublisher");
}

template <typename T>
void BusPublisher<T>::runAsync()
{
    if (running_) {
        return;
    }

    running_ = true;
    comm_thread_ = std::unique_ptr<std::thread> { new std::thread(std::bind(&BusPublisher::communicationThread, this)) };
    status_thread_ = std::unique_ptr<std::thread> { new std::thread(std::bind(&BusPublisher::statusThread, this)) };
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
    log(INFO, "BusPublisher status thread started");

    while (running_) {
        publishStatus();

        std::unique_lock<std::mutex> lk { mtx_status_thread_ };
        cv_status_thread_.wait_for(lk, status_interval_, [this] { return !running_; });
    }

    log(INFO, "BusPublisher status thread finished");
}

template <typename T>
void BusPublisher<T>::communicationThread()
{
    log(INFO, "BusPublisher communication thread started");

    while (running_) {
        try {
            auto sock = nng::pub::open();
            sock.listen(url_.c_str());
            log(INFO, "BusPublisher publishing to '{}'", url_);

            for (;;) {
                std::unique_lock<std::mutex> lk { mtx_publish_queue_ };

                while (!publish_queue_.empty()) {
                    sock.send(nng::view { &publish_queue_.front(), sizeof(message_type) });
                    publish_queue_.pop_front();
                }

                if (!running_) {
                    break;
                } else {
                    cv_publish_queue_.wait(lk, [this] { return !running_ || !publish_queue_.empty(); });
                }
            }
        } catch (const nng::exception& e) {
            log(ERROR, "BusPublisher caught error: {}: {}", e.who(), e.what());
            std::this_thread::sleep_for(reconnect_interval_);
        }
    }

    log(INFO, "BusPublisher communication thread finished");
}

template class BusPublisher<DaqoniteStateMessage>;
template class BusPublisher<DaqontrolStateMessage>;
template class BusPublisher<DaqsitterStateMessage>;
