#pragma once

#include <tinyfsm.hpp>

namespace Daqontrol {
namespace events {
    struct Disconnected : tinyfsm::Event {
    };
    struct Connected : tinyfsm::Event {
    };
    struct Idle : tinyfsm::Event {
    };
    struct Configured : tinyfsm::Event {
    };
    struct Started : tinyfsm::Event {
    };
}
}
