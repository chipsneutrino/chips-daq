#pragma once

#include <util/control_msg.h>
#include <util/publisher.h>

namespace ControlBus {
class BusMaster : public Publisher<ControlMessage> {
protected:
    void connected() override;
    void disconnected(const nng::exception& e) override;

public:
    explicit BusMaster() = default;
    virtual ~BusMaster() = default;
};
}