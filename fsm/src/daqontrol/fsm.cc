#include "daqontrol/fsm.h"
#include "daqontrol/states.h"

namespace Daqontrol {
void FSM::react(events::Disconnected const& e)
{
    transit<states::Offline>();
}

void FSM::react(events::Connected const& e)
{
    transit<states::Unknown>();
}
}

FSM_INITIAL_STATE(Daqontrol::FSM, Daqontrol::states::Offline)
