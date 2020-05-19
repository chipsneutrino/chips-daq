#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include <util/bus_publisher.h>
#include <util/control_msg.h>

#include "daq_control.h"

class DaqontrolPublisher : public BusPublisher<DaqontrolStateMessage> {
public:
    explicit DaqontrolPublisher(std::shared_ptr<DAQControl> daq_control);
    virtual ~DaqontrolPublisher() = default;

private:
    std::shared_ptr<DAQControl> daq_control_;

    void publishStatus();
};
