#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>

#include "badgerboard.h"
#include "badgerboard_datagrams.h"

Badgerboard::Badgerboard()
    : Logging {}
    , address_ { "tcp://192.168.0.61:54321" } // FIXME: make this configurable
    , req_sock_ { nng::req::open() }
    , req_mutex_ {}
{
    setUnitName("Badgerboard[{}]", address_);
    req_sock_.dial(address_.c_str());
}

bool Badgerboard::blockingSend(nng::msg&& request_msg)
{
    std::lock_guard<std::mutex> l { req_mutex_ };
    const std::uint8_t sent_request_type { request_msg.body().data<BadgerboardCommonHeader>()->type };
    nng::msg response_msg {};

    log(DEBUG, "Sending message of type {} and size {} bytes.", (std::uint8_t)sent_request_type, request_msg.body().size());

    try {
        // TODO: timeout?
        req_sock_.send(std::move(request_msg));
        response_msg = req_sock_.recv_msg();
    } catch (const nng::exception& e) {
        // TODO: print stuff
        return false;
    }

    if (response_msg.body().size() != sizeof(BadgerboardResponse)) {
        // TODO: throw stuff
        return false;
    }

    const auto& response { *response_msg.body().data<BadgerboardResponse>() };
    if (response.request_type != sent_request_type) {
        // TODO: throw stuff
        return false;
    }

    if (response.response_type != static_cast<std::uint8_t>(BadgerboardResponseType::Ack)) {
        // TODO: throw stuff
        return false;
    }

    return true;
}

bool Badgerboard::configureHub(const char* config, std::size_t config_size)
{
    return composeAndSendRequest<BadgerboardConfigureHubDatagramHeader>(
        BadgerboardRequestType::ConfigureHub, config_size,
        [&](BadgerboardConfigureHubDatagramHeader& header, char* body) {
            header.config_size = config_size;
            std::memcpy(body, config, config_size);
        });
}

bool Badgerboard::configureRun(const char* config, std::size_t config_size)
{
    return composeAndSendRequest<BadgerboardConfigureRunDatagramHeader>(
        BadgerboardRequestType::ConfigureRun, config_size,
        [&](BadgerboardConfigureRunDatagramHeader& header, char* body) {
            header.config_size = config_size;
            std::memcpy(body, config, config_size);
        });
}

bool Badgerboard::setPowerState(const bool channel_powered[N_CHANNELS])
{
    return composeAndSendRequest<BadgerboardSetPowerStateDatagramHeader>(
        BadgerboardRequestType::SetPowerState, 0,
        [&](BadgerboardSetPowerStateDatagramHeader& header, char* body) {
            header.channel_bitfield = composeBitfield(channel_powered);
        });
}

bool Badgerboard::reprogram(const bool channel_reprogrammed[N_CHANNELS], const char* firmware, std::size_t firmware_size)
{
    return composeAndSendRequest<BadgerboardReprogramDatagramHeader>(
        BadgerboardRequestType::Reprogram, firmware_size,
        [&](BadgerboardReprogramDatagramHeader& header, char* body) {
            header.channel_bitfield = composeBitfield(channel_reprogrammed);
            header.firmware_size = firmware_size;
            std::memcpy(body, firmware, firmware_size);
        });
}

std::uint16_t Badgerboard::composeBitfield(const bool channels[N_CHANNELS])
{
    std::uint16_t bitfield { 0 };
    for (std::size_t channel_idx = 0; channel_idx < N_CHANNELS; ++channel_idx) {
        if (channels[channel_idx]) {
            bitfield |= 1 << channel_idx;
        }
    }

    return bitfield;
}

bool Badgerboard::beginDataRun()
{
    return composeAndSendRequest<BadgerboardBeginDataRunDatagramHeader>(BadgerboardRequestType::BeginDataRun);
}

bool Badgerboard::abortDataRun()
{
    return composeAndSendRequest<BadgerboardAbortDataRunDatagramHeader>(BadgerboardRequestType::AbortDataRun);
}

bool Badgerboard::terminate()
{
    return composeAndSendRequest<BadgerboardTerminateDatagramHeader>(BadgerboardRequestType::Terminate);
}

bool Badgerboard::shutdown()
{
    return composeAndSendRequest<BadgerboardShutdownDatagramHeader>(BadgerboardRequestType::Shutdown);
}
