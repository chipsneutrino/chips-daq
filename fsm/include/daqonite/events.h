#pragma once

#include <tinyfsm.hpp>

namespace Daqonite {
namespace events {
    struct Disconnected : tinyfsm::Event {
    };
    struct Connected : tinyfsm::Event {
    };
    struct Ready : tinyfsm::Event {
    };
    struct Running : tinyfsm::Event {
    };
}
}
