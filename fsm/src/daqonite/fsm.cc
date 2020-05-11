#include "daqonite/fsm.h"
#include "daqonite/states.h"

namespace Daqonite {
FSM::FSM()
    : Fsm<FSM> {}
    , Logging {}
{
    setUnitName("Daqonite");
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

FSM_INITIAL_STATE(Daqonite::FSM, Daqonite::states::Offline)
