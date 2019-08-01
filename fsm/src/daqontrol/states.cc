#include <util/elastic_interface.h>

#include "daqontrol/states.h"
#include "global.h"
#include "global_events.h"

namespace Daqontrol {
namespace states {
    void Offline::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Offline");
        g_elastic.state("daqontrol", "Offline");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Unknown");
        g_elastic.state("daqontrol", "Unknown");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::react(events::Initialising const& e)
    {
        transit<states::Initialising>();
    }

    void Unknown::react(events::Ready const& e)
    {
        transit<states::Ready>();
    }

    void Unknown::react(events::Configured const& e)
    {
        transit<states::Configured>();
    }

    void Unknown::react(events::Started const& e)
    {
        transit<states::Started>();
    }

    void Initialising::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Initialising");
        g_elastic.state("daqontrol", "Initialising");
        global.sendEvent(StateUpdate{});
    }

    void Initialising::react(events::Ready const& e)
    {
        transit<states::Ready>();
    }

    void Ready::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Ready");
        g_elastic.state("daqontrol", "Ready");
        global.sendEvent(StateUpdate{});
    }

    void Ready::react(events::Configured const& e)
    {
        transit<states::Configured>();
    }

    void Configured::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Configured");
        g_elastic.state("daqontrol", "Configured");
        global.sendEvent(StateUpdate{});
    }

    void Configured::react(events::Started const& e)
    {
        transit<states::Started>();
    }

    void Started::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Started");
        g_elastic.state("daqontrol", "Started");
        global.sendEvent(StateUpdate{});
    }

    void Started::react(events::Configured const& e)
    {
        transit<states::Configured>();
    }
}
}