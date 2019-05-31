#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>

#include <nngpp/protocol/pub0.h>

#include <util/async_component.h>

template <typename MessageType>
class Publisher : public AsyncComponent {
public:
    using message_type = MessageType;

protected:
    virtual void connected() {}

    virtual void disconnected(const nng::exception& e) {}

    virtual void run() override
    {
        while (running_) {
            try {
                auto sock = nng::pub::open();
                sock.listen(message_type::URL);

                connected();

                while (running_) {
                    std::unique_lock<std::mutex> lk{ mtx_publish_queue_ };

                    // We now have exclusive access to the queue, publish as many messages as possible.
                    while (!publish_queue_.empty()) {
                        sock.send(nng::view{ &publish_queue_.front(), sizeof(message_type) });
                        publish_queue_.pop_front();
                    }

                    if (!running_) {
                        // Termination requested.
                        break;
                    } else {
                        // Nothing to do, suspend thread until termination / new message arrives.
                        cv_publish_queue_.wait(lk, [this] { return !running_ || !publish_queue_.empty(); });
                    }
                }
            } catch (const nng::exception& e) {
                disconnected(e);
                std::this_thread::sleep_for(reconnect_interval_);
            }
        }
    }

public:
    explicit Publisher()
        : AsyncComponent{}
        , publish_queue_{}
        , cv_publish_queue_{}
        , mtx_publish_queue_{}
        , reconnect_interval_{ 2000 }
    {
    }

    virtual ~Publisher() = default;

    void publish(message_type&& message)
    {
        std::lock_guard<std::mutex> lk{ mtx_publish_queue_ };
        publish_queue_.emplace_back(std::move(message));
        cv_publish_queue_.notify_one();
    }

    virtual void notifyJoin() override
    {
        AsyncComponent::notifyJoin();
        cv_publish_queue_.notify_one();
    }

private:
    std::list<message_type> publish_queue_;
    std::condition_variable cv_publish_queue_;
    std::mutex mtx_publish_queue_;

    std::chrono::milliseconds reconnect_interval_;
};