#pragma once

#include <tinyfsm.hpp>

#include "control_bus/fsm.h"
#include "daqonite/fsm.h"
#include "experiment/fsm.h"
#include "daqontrol/fsm.h"

using MainFSM = tinyfsm::FsmList<
    Experiment::FSM,
    ControlBus::FSM,
    Daqonite::FSM,
    Daqontrol::FSM>;
