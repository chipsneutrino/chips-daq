#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include "daq_handler.h"
#include <util/control_msg.h>
#include <util/bus_publisher.h>

class DaqonitePublisher : public BusPublisher<DaqoniteStateMessage> {
public:

    explicit DaqonitePublisher(std::shared_ptr<DAQHandler> daq_handler, const std::string& url);
    virtual ~DaqonitePublisher() = default;

private:
    std::shared_ptr<DAQHandler> daq_handler_;

    void publishStatus();
};
