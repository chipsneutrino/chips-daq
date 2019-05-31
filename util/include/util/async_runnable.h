#pragma once

class AsyncRunnable {
public:
    explicit AsyncRunnable() = default;
    virtual ~AsyncRunnable() = default;

    virtual void runAsync() = 0;
    virtual void notifyJoin() = 0;
    virtual void join() = 0;
};
