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

#include "monitoring_handler.h"

class DaqsitterPublisher : public BusPublisher<DaqsitterStateMessage> {
public:
    explicit DaqsitterPublisher(std::shared_ptr<MonitoringHandler> monitoring_handler, const std::string& url);
    virtual ~DaqsitterPublisher() = default;

private:
    std::shared_ptr<MonitoringHandler> monitoring_handler_;

    void publishStatus();
};
