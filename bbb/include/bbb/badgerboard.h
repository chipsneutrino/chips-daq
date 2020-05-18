#pragma once

#include <cstdint>
#include <mutex>
#include <string>

#include <nngpp/nngpp.h>

class Badgerboard {
public:
    explicit Badgerboard();

    static constexpr std::size_t N_CHANNELS { 16 };

    bool configureHub();
    bool configureRun();
    bool setPowerState(const bool channel_powered[N_CHANNELS]);
    bool reprogram();
    bool beginDataRun();
    bool abortDataRun();
    bool terminate();
    bool shutdown();

private:
    bool sendAndWaitForAcknowledgement(nng::msg&& request_msg);

    std::string address_;

    std::mutex req_mutex_;
    nng::socket req_sock_;

    static std::uint16_t composeBitfield(const bool channels[N_CHANNELS]);
};