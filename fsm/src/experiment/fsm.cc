#include "experiment/fsm.h"
#include "experiment/states.h"

namespace Experiment {
FSM::FSM()
    : Fsm<FSM> {}
    , Logging {}
{
    setUnitName("Experiment");
}

void FSM::react(KillSignal const& e)
{
    transit<states::Exit>();
}
}

FSM_INITIAL_STATE(Experiment::FSM, Experiment::states::Init)
