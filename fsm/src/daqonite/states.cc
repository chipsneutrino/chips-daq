#include <util/elastic_interface.h>

#include "daqonite/states.h"
#include "global.h"
#include "global_events.h"

namespace Daqonite {
namespace states {
    void Offline::entry()
    {
        g_elastic.log(INFO, "Daqonite : Offline");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::entry()
    {
        g_elastic.log(INFO, "Daqonite : Unknown");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::react(events::Idle const& e)
    {
        transit<states::Idle>();
    }

    void Unknown::react(events::RunInProgress const& e)
    {
        transit<states::RunInProgress>();
    }

    void Idle::entry()
    {
        g_elastic.log(INFO, "Daqonite : Idle");
        global.sendEvent(StateUpdate{});
    }

    void Idle::react(events::RunInProgress const& e)
    {
        transit<states::RunInProgress>();
    }

    void RunInProgress::entry()
    {
        g_elastic.log(INFO, "Daqonite : RunInProgress");
        global.sendEvent(StateUpdate{});
    }

    void RunInProgress::react(events::Idle const& e)
    {
        transit<states::Idle>();
    }
}
}