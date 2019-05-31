#include <functional>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>

#include "bus_publisher.h"
#include <util/elastic_interface.h>

BusPublisher::BusPublisher(std::shared_ptr<DAQHandler> daq_handler)
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
    , daq_handler_{ std::move(daq_handler) }
{
}

void BusPublisher::runAsync()
{
    if (running_) {
        return;
    }

    running_ = true;
    comm_thread_ = std::unique_ptr<std::thread>{ new std::thread(std::bind(&BusPublisher::communicationThread, this)) };
    status_thread_ = std::unique_ptr<std::thread>{ new std::thread(std::bind(&BusPublisher::statusThread, this)) };
}

void BusPublisher::join()
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

void BusPublisher::statusThread()
{
    g_elastic.log(INFO, "BusPublisher status thread started");

    while (running_) {
        publishStatus();

        std::unique_lock<std::mutex> lk{ mtx_status_thread_ };
        cv_status_thread_.wait_for(lk, status_interval_, [this] { return !running_; });
    }

    g_elastic.log(INFO, "BusPublisher status thread finished");
}

void BusPublisher::publishStatus()
{
    // TODO: synchronize?
    message_type message{};

    if (daq_handler_->getMode()) {
        message.Discriminator = DaqoniteStateMessage::Mining::Discriminator;
        message.Payload.pMining = DaqoniteStateMessage::Mining{};
        message.Payload.pMining.Which = daq_handler_->getRunType();
    } else {
        message.Discriminator = DaqoniteStateMessage::Idle::Discriminator;
    }

    std::lock_guard<std::mutex> lk{ mtx_publish_queue_ };
    publish_queue_.emplace_back(std::move(message));
    cv_publish_queue_.notify_one();
}

void BusPublisher::communicationThread()
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
