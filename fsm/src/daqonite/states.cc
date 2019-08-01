#include <util/elastic_interface.h>

#include "daqonite/states.h"
#include "global.h"
#include "global_events.h"

namespace Daqonite {
namespace states {
    void Offline::entry()
    {
        g_elastic.log(INFO, "Daqonite : Offline");
        g_elastic.state("daqonite", "Offline");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::entry()
    {
        g_elastic.log(INFO, "Daqonite : Unknown");
        g_elastic.state("daqonite", "Unknown");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::react(events::Ready const& e)
    {
        transit<states::Ready>();
    }

    void Unknown::react(events::Running const& e)
    {
        transit<states::Running>();
    }

    void Ready::entry()
    {
        g_elastic.log(INFO, "Daqonite : Ready");
        g_elastic.state("daqonite", "Ready");
        global.sendEvent(StateUpdate{});
    }

    void Ready::react(events::Running const& e)
    {
        transit<states::Running>();
    }

    void Running::entry()
    {
        g_elastic.log(INFO, "Daqonite : Running");
        g_elastic.state("daqonite", "Running");
        global.sendEvent(StateUpdate{});
    }

    void Running::react(events::Ready const& e)
    {
        transit<states::Ready>();
    }
}
}