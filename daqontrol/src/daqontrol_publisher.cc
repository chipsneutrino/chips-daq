#include <functional>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>
#include <util/elastic_interface.h>

#include "daqontrol_publisher.h"

DaqontrolPublisher::DaqontrolPublisher(std::shared_ptr<DAQControl> daq_control, const std::string& url)
    : BusPublisher { url }
    , daq_control_ { std::move(daq_control) }
{
}

void DaqontrolPublisher::publishStatus()
{
    message_type message {};

    if (daq_control_->getMode() == Control::Ready) {
        message.Discriminator = DaqontrolStateMessage::Ready::Discriminator;
    } else if (daq_control_->getMode() == Control::Configured) {
        message.Discriminator = DaqontrolStateMessage::Configured::Discriminator;
    } else if (daq_control_->getMode() == Control::Started) {
        message.Discriminator = DaqontrolStateMessage::Started::Discriminator;
    }

    std::lock_guard<std::mutex> lk { mtx_publish_queue_ };
    publish_queue_.emplace_back(std::move(message));
    cv_publish_queue_.notify_one();
}