#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>

#include "badgerboard.h"
#include "badgerboard_datagrams.h"

Badgerboard::Badgerboard()
    : address_ { "tcp://192.168.0.61:54321" } // FIXME: make this configurable
    , req_sock_ { nng::req::open() }
    , req_mutex_ {}
{
    req_sock_.dial(address_.c_str());
}

bool Badgerboard::sendAndWaitForAcknowledgement(nng::msg&& request_msg)
{
    std::lock_guard<std::mutex> l { req_mutex_ };
    const BadgerboardRequestType sent_request_type { request_msg.body().data<BadgerboardCommonHeader>()->type };
    nng::msg response_msg {};

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

    if (response.response_type != BadgerboardResponseType::Ack) {
        // TODO: throw stuff
        return false;
    }

    return true;
}

bool Badgerboard::configureHub()
{
    // TODO: implement me
    return false;
}

bool Badgerboard::configureRun()
{
    // TODO: implement me
    return false;
}

bool Badgerboard::setPowerState(const bool channel_powered[N_CHANNELS])
{
    nng::msg request_msg { sizeof(BadgerboardSetPowerStateDatagramHeader) };
    auto& request { *request_msg.body().data<BadgerboardSetPowerStateDatagramHeader>() };

    request.common.type = BadgerboardRequestType::SetPowerState;
    request.channel_bitfield = composeBitfield(channel_powered);

    return sendAndWaitForAcknowledgement(std::move(request_msg));
}

bool Badgerboard::reprogram()
{
    // TODO: implement me
    return false;
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
    nng::msg request_msg { sizeof(BadgerboardBeginDataRunDatagramHeader) };
    auto& request { *request_msg.body().data<BadgerboardBeginDataRunDatagramHeader>() };

    request.common.type = BadgerboardRequestType::BeginDataRun;

    return sendAndWaitForAcknowledgement(std::move(request_msg));
}

bool Badgerboard::abortDataRun()
{
    nng::msg request_msg { sizeof(BadgerboardAbortDataRunDatagramHeader) };
    auto& request { *request_msg.body().data<BadgerboardAbortDataRunDatagramHeader>() };

    request.common.type = BadgerboardRequestType::AbortDataRun;

    return sendAndWaitForAcknowledgement(std::move(request_msg));
}

bool Badgerboard::terminate()
{
    nng::msg request_msg { sizeof(BadgerboardTerminateDatagramHeader) };
    auto& request { *request_msg.body().data<BadgerboardTerminateDatagramHeader>() };

    request.common.type = BadgerboardRequestType::Terminate;

    return sendAndWaitForAcknowledgement(std::move(request_msg));
}

bool Badgerboard::shutdown()
{
    nng::msg request_msg { sizeof(BadgerboardShutdownDatagramHeader) };
    auto& request { *request_msg.body().data<BadgerboardShutdownDatagramHeader>() };

    request.common.type = BadgerboardRequestType::Shutdown;

    return sendAndWaitForAcknowledgement(std::move(request_msg));
}