#pragma once

#include <tinyfsm.hpp>

#include <util/control_msg.h>
#include <util/async_component.h>

namespace nng {
struct socket;
}

struct OpsMessage;

class OpsUplink : public AsyncComponent {
protected:
    void run() override;

public:
    explicit OpsUplink() = default;
    virtual ~OpsUplink() = default;

private:
    void handleMessage(nng::socket& sock, const OpsMessage& message);
    void acknowledge(nng::socket& sock, bool ok);
};

namespace OpsCommands {
struct Config : tinyfsm::Event {
};
struct StartData : tinyfsm::Event {
};
struct StopData : tinyfsm::Event {
};
struct StartRun : tinyfsm::Event {
    RunType type;       ///< Default to a normal test run
    float flasher_v;    ///< Default to 6.0 volts
};
struct StopRun : tinyfsm::Event {
};
}
