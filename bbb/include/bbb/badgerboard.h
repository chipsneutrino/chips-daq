#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

#include <nngpp/nngpp.h>

#include <util/logging.h>

class Badgerboard : protected Logging {
public:
    explicit Badgerboard();

    static constexpr std::size_t N_CHANNELS { 16 };

    bool configureHub(const char* config, std::size_t config_size);
    bool configureRun(const char* config, std::size_t config_size);
    bool setPowerState(const bool channel_powered[N_CHANNELS]);
    bool reprogram(const bool channel_reprogrammed[N_CHANNELS], const char* firmware, std::size_t firmware_size);
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
        header.common.seq_number = ++next_sequence_number_;
        config_ftor(header, body);

        return blockingSend(std::move(request_msg));
    }

    std::string address_;

    std::mutex req_mutex_;
    nng::socket req_sock_;

    std::uint32_t next_sequence_number_;

    static std::uint16_t composeBitfield(const bool channels[N_CHANNELS]);
};