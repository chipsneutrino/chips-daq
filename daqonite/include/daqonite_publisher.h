#pragma once

#include <memory>

#include <util/bus_publisher.h>
#include <util/control_msg.h>

#include "daq_handler.h"

class DaqonitePublisher : public BusPublisher<DaqoniteStateMessage> {
public:
    explicit DaqonitePublisher(std::shared_ptr<DAQHandler> daq_handler);
    virtual ~DaqonitePublisher() = default;

private:
    std::shared_ptr<DAQHandler> daq_handler_;

    void publishStatus();
};
