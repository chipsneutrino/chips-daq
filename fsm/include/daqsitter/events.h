#pragma once

#include <tinyfsm.hpp>

namespace Daqsitter {
namespace events {
    struct Disconnected : tinyfsm::Event {
    };
    struct Connected : tinyfsm::Event {
    };
    struct Ready : tinyfsm::Event {
    };
    struct Started : tinyfsm::Event {
    };
}
}
