#include <thread>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/sub0.h>

#include "badgerboard_datagrams.h"
#include "badgerboard_heartbeat.h"

// TODO: make configurable
static constexpr nng_duration HEARTBEAT_RECV_TIMEOUT { 1000 }; // ms
static constexpr std::chrono::milliseconds HEARTBEAT_DEAD_THRESHOLD { 5000 }; // ms
static constexpr std::chrono::milliseconds HEARTBEAT_HOLDOFF_INTERVAL { 30000 }; // ms

BadgerboardHeartbeat::BadgerboardHeartbeat()
    : Logging {}
    , address_ { "tcp://192.168.0.61:56113" } // FIXME: make this configurable
    , state_mutex_ {}
    , state_cv_ {}
    , state_ {}
{
    setUnitName("BadgerboardHeartbeat[{}]", address_);

    state_.connection = BadgerboardConnectionState::Offline;
}

void BadgerboardHeartbeat::run()
{
    modifyState([](BadgerboardState& state) {
        state.connection = BadgerboardConnectionState::Connecting;
    });

    while (running_) {
        Clock::time_point last_time_received { Clock::now() };

        try {
            auto sock { nng::sub::open() };
            nng::sub::set_opt_subscribe(sock, "");
            nng::set_opt_recv_timeout(sock, HEARTBEAT_RECV_TIMEOUT);
            sock.dial(address_.c_str());

            std::uint32_t next_seq_number { 0 };
            last_time_received = Clock::now();

            nng::msg msg {};
            while (running_) {
                msg = sock.recv_msg();

                if (msg.body().size() != sizeof(BadgerboardHeartbeatDatagram)) {
                    log(WARNING, "Unexpected heartbeat datagram size (expected {} bytes, got {})",
                        sizeof(BadgerboardHeartbeatDatagram), msg.body().size());
                    continue;
                }

                auto& heartbeat { *msg.body().data<BadgerboardHeartbeatDatagram>() };
                if (heartbeat.seq_number < next_seq_number) {
                    log(WARNING, "Skipping out-of-order heartbeat (expected sequence number at least {}, got {})",
                        next_seq_number, heartbeat.seq_number);
                    continue;
                }

                last_time_received = Clock::now();
                next_seq_number = 1 + heartbeat.seq_number;
                processHeartbeat(heartbeat);
            }
        } catch (const nng::exception& e) {
            using namespace std::chrono;

            bool is_dead {};
            if (e.get_error() == nng::error::timedout) {
                const auto silent_time { duration_cast<milliseconds>(Clock::now() - last_time_received) };
                is_dead = silent_time > HEARTBEAT_DEAD_THRESHOLD;
            } else {
                is_dead = true;
                log(WARNING, "Network error waiting for heartbeat - {}", e.what());
            }

            if (is_dead) {
                log(DEBUG, "Heartbeat silent for too long, marking device as offline");
                modifyState([](BadgerboardState& state) {
                    state.connection = BadgerboardConnectionState::Offline;
                });

                std::this_thread::sleep_for(HEARTBEAT_HOLDOFF_INTERVAL);

                log(DEBUG, "Attempting to reconnect");
                modifyState([](BadgerboardState& state) {
                    state.connection = BadgerboardConnectionState::Connecting;
                });
            }
        }
    }

    // This should unblock threads that are still waiting for a state change
    modifyState([](BadgerboardState& state) {
        state.connection = BadgerboardConnectionState::Offline;
    });
}

void BadgerboardHeartbeat::processHeartbeat(const BadgerboardHeartbeatDatagram& heartbeat)
{
    modifyState([&](BadgerboardState& state) {
        state.connection = BadgerboardConnectionState::Online;

        state.hubConfig = static_cast<BadgerboardConfigState>(heartbeat.hub_config);
        state.runConfig = static_cast<BadgerboardConfigState>(heartbeat.run_config);

        state.run = static_cast<BadgerboardDataRunState>(heartbeat.run);

        for (std::size_t channel_idx = 0; channel_idx < N_BADGERBOARD_CHANNELS; ++channel_idx) {
            state.channels[channel_idx] = static_cast<BadgerboardChannelState>(heartbeat.channel_states[channel_idx]);
        }

        // TODO: update channel state and sensors
    });
}

void BadgerboardHeartbeat::modifyState(StateModifier modifier)
{
    std::unique_lock<std::mutex> l { state_mutex_ };
    modifier(state_);
    state_cv_.notify_all();
}

void BadgerboardHeartbeat::waitForStateChange(std::chrono::milliseconds timeout, Predicate predicate)
{
    const Clock::time_point timeout_threshold { Clock::now() + timeout };
    std::unique_lock<std::mutex> l { state_mutex_ };

    // Make sure we are connected before we start waiting.
    if (!state_cv_.wait_until(l, timeout_threshold, [this] { return running_ && state_.connection == BadgerboardConnectionState::Online; })) {
        throw std::runtime_error { "Connection never established" };
    }

    // Now that we are running and connected, wait for the main thing.
    auto my_predicate = [this, &predicate]() {
        return !running_
            || state_.connection != BadgerboardConnectionState::Online
            || predicate(state_);
    };

    if (!state_cv_.wait_until(l, timeout_threshold, my_predicate)) {
        throw std::runtime_error { "Timed out" };
    }

    if (!running_) {
        throw std::runtime_error { "Stopped while waiting for a state change" };
    } else if (state_.connection != BadgerboardConnectionState::Online) {
        throw std::runtime_error { fmt::format("Heartbeat lost (connection state {})",
            static_cast<std::uint32_t>(state_.connection)) };
    } else {
        return; // Predicate satisfied.
    }
}
