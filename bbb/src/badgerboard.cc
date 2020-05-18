#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>

#include "badgerboard.h"
#include "badgerboard_datagrams.h"

Badgerboard::Badgerboard()
    : address_ { "tcp://192.168.0.61:54321" } // FIXME: make this configurable
{
}

void Badgerboard::sendAndWaitForAcknowledgement(nng::buffer& request)
{
    // TODO: exceptions may be thrown below, handle them

    const BadgerboardRequestType sent_request_type { request.data<BadgerboardCommonHeader>()->type };

    nng::socket req_sock { nng::req::open() };
    req_sock.dial(address_.c_str());
    req_sock.send(request);

    // TODO: timeout?

    nng::buffer response_buf { req_sock.recv() };

    if (response_buf.size() != sizeof(BadgerboardResponse)) {
        // TODO: throw stuff
        return;
    }

    const auto& response { *response_buf.data<BadgerboardResponse>() };
    if (response.request_type != sent_request_type) {
        // TODO: throw stuff
        return;
    }

    if (response.response_type != BadgerboardResponseType::Ack) {
        // TODO: throw stuff
    }
}

void Badgerboard::configureHub()
{
    // TODO: implement me
}

void Badgerboard::configureRun()
{
    // TODO: implement me
}

void Badgerboard::setPowerState()
{
    // TODO: implement me
}

void Badgerboard::reprogram()
{
    // TODO: implement me
}

void Badgerboard::beginDataRun()
{
    nng::buffer request_buf { sizeof(BadgerboardBeginDataRunDatagramHeader) };
    auto& request { *request_buf.data<BadgerboardBeginDataRunDatagramHeader>() };

    request.common.type = BadgerboardRequestType::BeginDataRun;

    sendAndWaitForAcknowledgement(request_buf);
}

void Badgerboard::abortDataRun()
{
    // TODO: implement me
}

void Badgerboard::terminate()
{
    // TODO: implement me
}

void Badgerboard::shutdown()
{
    // TODO: implement me
}