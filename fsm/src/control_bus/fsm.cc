#include "control_bus/fsm.h"
#include "control_bus/states.h"

namespace ControlBus {
FSM::FSM()
    : Fsm<FSM> {}
    , Logging {}
{
    setUnitName("ControlBus");
}
}

FSM_INITIAL_STATE(ControlBus::FSM, ControlBus::states::Offline)
