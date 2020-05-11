#include "daqsitter/fsm.h"
#include "daqsitter/states.h"

namespace Daqsitter {
FSM::FSM()
    : Fsm<FSM> {}
    , Logging {}
{
    setUnitName("Daqsitter");
}

void FSM::react(events::Disconnected const& e)
{
    transit<states::Offline>();
}

void FSM::react(events::Connected const& e)
{
    transit<states::Unknown>();
}
}

FSM_INITIAL_STATE(Daqsitter::FSM, Daqsitter::states::Offline)
