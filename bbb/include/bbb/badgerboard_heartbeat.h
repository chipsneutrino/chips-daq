#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

#include <nngpp/nngpp.h>

#include <util/async_component.h>
#include <util/logging.h>

#include <bbb/badgerboard_state.h>

struct BadgerboardHeartbeatDatagram;

class BadgerboardHeartbeat : protected Logging, public AsyncComponent {
public:
    explicit BadgerboardHeartbeat();

    using Predicate = std::function<bool(const BadgerboardState&)>;
    void waitForStateChange(std::chrono::milliseconds timeout, Predicate predicate);

protected:
    void run() override;

private:
    using Clock = std::chrono::high_resolution_clock;

    std::string address_;

    std::mutex state_mutex_;
    std::condition_variable state_cv_;
    BadgerboardState state_;

    void processHeartbeat(const BadgerboardHeartbeatDatagram& heartbeat);

    using StateModifier = std::function<void(BadgerboardState&)>;
    void modifyState(StateModifier modifier);
};