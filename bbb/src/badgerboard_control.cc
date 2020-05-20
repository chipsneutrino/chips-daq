#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>

#include "badgerboard_control.h"
#include "badgerboard_datagrams.h"

BadgerboardControl::BadgerboardControl()
    : Logging {}
    , req_address_ { "tcp://192.168.0.61:56114" } // FIXME: make this configurable
    , req_sock_ { nng::req::open() }
    , req_mutex_ {}
    , req_next_sequence_number_ { 0 }
{
    setUnitName("BadgerboardControl[{}]", req_address_);

    req_sock_.dial(req_address_.c_str()); // TODO: exception?
}

bool BadgerboardControl::blockingSend(nng::msg&& request_msg)
{
    std::lock_guard<std::mutex> l { req_mutex_ };

    // Peek at the header, so that we know what response to receive
    const auto& common_header { *request_msg.body().data<BadgerboardCommonHeader>() };
    const std::uint8_t sent_request_type { common_header.type };
    const std::uint32_t seq_number { common_header.seq_number };

    nng::msg response_msg {};

    try {
        // TODO: timeout?
        req_sock_.send(std::move(request_msg));
        response_msg = req_sock_.recv_msg();
    } catch (const nng::exception& e) {
        log(WARNING, "Error sending a request of type {} - {}",
            sent_request_type, e.what());
        return false;
    }

    if (response_msg.body().size() != sizeof(BadgerboardResponse)) {
        log(WARNING, "Unexpected response size (expected {} bytes, received {})",
            sizeof(BadgerboardResponse), response_msg.body().size());
        return false;
    }

    const auto& response { *response_msg.body().data<BadgerboardResponse>() };
    if (response.request_type != sent_request_type) {
        log(WARNING, "Unexpected response type (expected {}, received {})",
            sent_request_type, response.request_type);
        return false;
    }

    if (response.seq_number != seq_number) {
        log(WARNING, "Unexpected response sequence number (expected {}, received {})",
            seq_number, response.seq_number);
        return false;
    }

    if (response.response_type != static_cast<std::uint8_t>(BadgerboardResponseType::Ack)) {
        log(WARNING, "Received NAK for a request of type {}", sent_request_type);
        return false;
    }

    return true;
}

bool BadgerboardControl::configureHub(const char* config, std::size_t config_size)
{
    log(DEBUG, "Configuring hub");
    return composeAndSendRequest<BadgerboardConfigureHubDatagramHeader>(
        BadgerboardRequestType::ConfigureHub, config_size,
        [&](BadgerboardConfigureHubDatagramHeader& header, char* body) {
            header.config_size = config_size;
            std::memcpy(body, config, config_size);
        });
}

bool BadgerboardControl::configureRun(const char* config, std::size_t config_size)
{
    log(DEBUG, "Configuring run");
    return composeAndSendRequest<BadgerboardConfigureRunDatagramHeader>(
        BadgerboardRequestType::ConfigureRun, config_size,
        [&](BadgerboardConfigureRunDatagramHeader& header, char* body) {
            header.config_size = config_size;
            std::memcpy(body, config, config_size);
        });
}

bool BadgerboardControl::setPowerState(const BadgerboardChannelSelection& powered_channels)
{
    log(DEBUG, "Setting power state");
    return composeAndSendRequest<BadgerboardSetPowerStateDatagramHeader>(
        BadgerboardRequestType::SetPowerState, 0,
        [&](BadgerboardSetPowerStateDatagramHeader& header, char* body) {
            header.channel_bitfield = composeBitfield(powered_channels);
        });
}

bool BadgerboardControl::reprogram(const BadgerboardChannelSelection& reprogrammed_channels, const char* firmware, std::size_t firmware_size)
{
    log(DEBUG, "Reprogramming");
    return composeAndSendRequest<BadgerboardReprogramDatagramHeader>(
        BadgerboardRequestType::Reprogram, firmware_size,
        [&](BadgerboardReprogramDatagramHeader& header, char* body) {
            header.channel_bitfield = composeBitfield(reprogrammed_channels);
            header.firmware_size = firmware_size;
            std::memcpy(body, firmware, firmware_size);
        });
}

std::uint16_t BadgerboardControl::composeBitfield(const BadgerboardChannelSelection& channels)
{
    std::uint16_t bitfield { 0 };
    for (std::size_t channel_idx = 0; channel_idx < N_BADGERBOARD_CHANNELS; ++channel_idx) {
        if (channels[channel_idx]) {
            bitfield |= 1 << channel_idx;
        }
    }

    return bitfield;
}

bool BadgerboardControl::beginDataRun()
{
    log(DEBUG, "Beginning data run");
    return composeAndSendRequest<BadgerboardBeginDataRunDatagramHeader>(BadgerboardRequestType::BeginDataRun);
}

bool BadgerboardControl::abortDataRun()
{
    log(DEBUG, "Aborting data run");
    return composeAndSendRequest<BadgerboardAbortDataRunDatagramHeader>(BadgerboardRequestType::AbortDataRun);
}

bool BadgerboardControl::terminate()
{
    log(DEBUG, "Terminating daemon");
    return composeAndSendRequest<BadgerboardTerminateDatagramHeader>(BadgerboardRequestType::Terminate);
}

bool BadgerboardControl::shutdown()
{
    log(DEBUG, "Shutting down device");
    return composeAndSendRequest<BadgerboardShutdownDatagramHeader>(BadgerboardRequestType::Shutdown);
}

bool BadgerboardControl::resetConfiguration()
{
    log(DEBUG, "Resetting configuration");
    return composeAndSendRequest<BadgerboardResetConfigurationDatagramHeader>(BadgerboardRequestType::ResetConfiguration);
}
