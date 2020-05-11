#pragma once

#include <tinyfsm.hpp>

#include <util/async_component.h>
#include <util/control_msg.h>
#include <util/logging.h>

namespace nng {
struct socket;
}

struct OpsMessage;

class OpsUplink : public AsyncComponent, protected Logging {
protected:
    void run() override;

public:
    explicit OpsUplink(const std::string& bus_url);
    virtual ~OpsUplink() = default;

private:
    void handleMessage(nng::socket& sock, const OpsMessage& message);
    void acknowledge(nng::socket& sock, bool ok);

    std::string bus_url_;
};

namespace OpsCommands {
struct Config : tinyfsm::Event {
    std::string config_file;
};
struct StartData : tinyfsm::Event {
};
struct StopData : tinyfsm::Event {
};
struct StartRun : tinyfsm::Event {
    RunType run_type;
};
struct StopRun : tinyfsm::Event {
};
struct Exit : tinyfsm::Event {
};
}
