#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include <util/async_runnable.h>

class AsyncComponent : public AsyncRunnable {
protected:
    virtual void run() = 0;

    std::atomic_bool running_;
    std::unique_ptr<std::thread> thread_;

public:
    explicit AsyncComponent();
    virtual ~AsyncComponent() = default;

    virtual void runAsync() override;
    virtual void notifyJoin() override;
    virtual void join() override;
};
