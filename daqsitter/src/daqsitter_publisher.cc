#include <functional>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>
#include <util/elastic_interface.h>

#include "daqsitter_publisher.h"

DaqsitterPublisher::DaqsitterPublisher(std::shared_ptr<MonitoringHandler> monitoring_handler, const std::string& url)
    : BusPublisher { url }
    , monitoring_handler_ { std::move(monitoring_handler) }
{
}

void DaqsitterPublisher::publishStatus()
{
    // TODO: synchronize?
    message_type message {};

    if (monitoring_handler_->getMode()) {
        message.Discriminator = DaqsitterStateMessage::Started::Discriminator;
    } else {
        message.Discriminator = DaqsitterStateMessage::Ready::Discriminator;
    }

    std::lock_guard<std::mutex> lk { mtx_publish_queue_ };
    publish_queue_.emplace_back(std::move(message));
    cv_publish_queue_.notify_one();
}