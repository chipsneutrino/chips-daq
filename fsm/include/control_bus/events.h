#pragma once

#include <tinyfsm.hpp>

namespace ControlBus {
namespace events {
    struct Disconnected : tinyfsm::Event {
    };
    struct Connected : tinyfsm::Event {
    };
}
}