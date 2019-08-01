#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include "monitoring_handler.h"
#include <util/control_msg.h>
#include <util/bus_publisher.h>

class DaqsitterPublisher : public BusPublisher<DaqsitterStateMessage> {
public:

    explicit DaqsitterPublisher(std::shared_ptr<MonitoringHandler> monitoring_handler);
    virtual ~DaqsitterPublisher() = default;

private:
    std::shared_ptr<MonitoringHandler> monitoring_handler_;

    void publishStatus();
};
