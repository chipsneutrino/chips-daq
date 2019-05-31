#pragma once

#include <tinyfsm.hpp>

namespace Daqonite {
namespace events {
    struct Disconnected : tinyfsm::Event {
    };
    struct Connected : tinyfsm::Event {
    };
    struct Idle : tinyfsm::Event {
    };
    struct RunInProgress : tinyfsm::Event {
    };
}
}
