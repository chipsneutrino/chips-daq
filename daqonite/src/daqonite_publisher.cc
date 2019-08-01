#include <functional>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>

#include "daqonite_publisher.h"
#include <util/elastic_interface.h>

DaqonitePublisher::DaqonitePublisher(std::shared_ptr<DAQHandler> daq_handler)
    : BusPublisher()
    , daq_handler_{ std::move(daq_handler) }
{
}

void DaqonitePublisher::publishStatus()
{
    // TODO: synchronize?
    message_type message{};

    if (daq_handler_->getMode()) {
        message.Discriminator = DaqoniteStateMessage::Running::Discriminator;
        message.Payload.pRunning = DaqoniteStateMessage::Running{};
        message.Payload.pRunning.Which = daq_handler_->getRunType();
    } else {
        message.Discriminator = DaqoniteStateMessage::Ready::Discriminator;
    }

    std::lock_guard<std::mutex> lk{ mtx_publish_queue_ };
    publish_queue_.emplace_back(std::move(message));
    cv_publish_queue_.notify_one();
}