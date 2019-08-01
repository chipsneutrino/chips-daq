#include <util/elastic_interface.h>

#include "daqsitter/states.h"
#include "global.h"
#include "global_events.h"

namespace Daqsitter {
namespace states {
    void Offline::entry()
    {
        g_elastic.log(INFO, "Daqsitter : Offline");
        g_elastic.state("daqsitter", "Offline");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::entry()
    {
        g_elastic.log(INFO, "Daqsitter : Unknown");
        g_elastic.state("daqsitter", "Unknown");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::react(events::Ready const& e)
    {
        transit<states::Ready>();
    }

    void Unknown::react(events::Started const& e)
    {
        transit<states::Started>();
    }

    void Ready::entry()
    {
        g_elastic.log(INFO, "Daqsitter : Ready");
        g_elastic.state("daqsitter", "Ready");
        global.sendEvent(StateUpdate{});
    }

    void Ready::react(events::Started const& e)
    {
        transit<states::Started>();
    }

    void Started::entry()
    {
        g_elastic.log(INFO, "Daqsitter : Started");
        g_elastic.state("daqsitter", "Started");
        global.sendEvent(StateUpdate{});
    }

    void Started::react(events::Ready const& e)
    {
        transit<states::Ready>();
    }
}
}