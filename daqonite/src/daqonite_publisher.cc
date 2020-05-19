#include <functional>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>

#include <util/config.h>
#include <util/logging.h>

#include "daqonite_publisher.h"

DaqonitePublisher::DaqonitePublisher(std::shared_ptr<DAQHandler> daq_handler)
    : BusPublisher { g_config.lookupString("bus.daqonite") }
    , daq_handler_ { std::move(daq_handler) }
{
}

void DaqonitePublisher::publishStatus()
{
    // TODO: synchronize?
    message_type message {};

    std::shared_ptr<DataRun> run { daq_handler_->getRun() };

    if (run && run->getState() == DataRunState::Running) {
        message.Discriminator = DaqoniteStateMessage::Running::Discriminator;
        message.Payload.pRunning = DaqoniteStateMessage::Running {};
        message.Payload.pRunning.Which = run->getType();
    } else {
        message.Discriminator = DaqoniteStateMessage::Ready::Discriminator;
    }

    std::lock_guard<std::mutex> lk { mtx_publish_queue_ };
    publish_queue_.emplace_back(std::move(message));
    cv_publish_queue_.notify_one();
}
