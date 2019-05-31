#pragma once

#include <tinyfsm.hpp>

struct KillSignal : tinyfsm::Event {
};

struct StateUpdate : tinyfsm::Event {
};
