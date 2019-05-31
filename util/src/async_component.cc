#include <functional>

#include "async_component.h"

AsyncComponent::AsyncComponent()
    : AsyncRunnable{}
    , running_{ false }
    , thread_{}
{
}

void AsyncComponent::runAsync()
{
    if (running_) {
        return;
    }

    running_ = true;
    thread_ = std::unique_ptr<std::thread>{ new std::thread(std::bind(&AsyncComponent::run, this)) }; // TODO: std::make_unique in c++14
}

void AsyncComponent::notifyJoin()
{
    if (!running_) {
        return;
    }

    running_ = false;
}

void AsyncComponent::join()
{
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }

    thread_.reset();
}
