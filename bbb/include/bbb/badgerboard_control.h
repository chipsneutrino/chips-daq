#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

#include <nngpp/nngpp.h>

#include <util/logging.h>

#include <bbb/constants.h>

using BadgerboardChannelSelection = std::array<bool, N_BADGERBOARD_CHANNELS>;

class BadgerboardControl : protected Logging {
public:
    explicit BadgerboardControl();

    void blockingCheckHeartbeat();

    bool resetConfiguration();
    bool configureHub(const char* config, std::size_t config_size);
    bool configureRun(const char* config, std::size_t config_size);
    bool setPowerState(const BadgerboardChannelSelection& powered_channels);
    bool reprogram(const BadgerboardChannelSelection& reprogrammed_channels, const char* firmware, std::size_t firmware_size);
    bool beginDataRun();
    bool abortDataRun();
    bool terminate();
    bool shutdown();

private:
    bool blockingSend(nng::msg&& request_msg);

    template <
        typename RequestHeaderType,
        typename RequestDiscriminator,
        typename RequestConfigFtor = std::function<void(RequestHeaderType&, char*)>>
    bool composeAndSendRequest(
        RequestDiscriminator request_type, std::size_t body_size = 0,
        RequestConfigFtor config_ftor = [](RequestHeaderType&, char*) {})
    {
        nng::msg request_msg { sizeof(RequestHeaderType) + body_size };
        auto& header { *request_msg.body().data<RequestHeaderType>() };
        auto body { reinterpret_cast<char*>(request_msg.body().data()) + sizeof(RequestHeaderType) };

        header.common.type = static_cast<decltype(header.common.type)>(request_type);
        header.common.seq_number = ++req_next_sequence_number_;
        config_ftor(header, body);

        return blockingSend(std::move(request_msg));
    }

    std::string req_address_;
    std::mutex req_mutex_;
    nng::socket req_sock_;
    std::uint32_t req_next_sequence_number_;

    static std::uint16_t composeBitfield(const BadgerboardChannelSelection& channels);
};