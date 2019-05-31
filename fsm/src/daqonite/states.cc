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

    void Unknown::react(events::Mining const& e)
    {
        transit<states::Mining>();
    }

    void Idle::entry()
    {
        g_elastic.log(INFO, "Daqonite : Idle");
        global.sendEvent(StateUpdate{});
    }

    void Idle::react(events::Mining const& e)
    {
        transit<states::Mining>();
    }

    void Mining::entry()
    {
        g_elastic.log(INFO, "Daqonite : Mining");
        global.sendEvent(StateUpdate{});
    }

    void Mining::react(events::Idle const& e)
    {
        transit<states::Idle>();
    }
}
}