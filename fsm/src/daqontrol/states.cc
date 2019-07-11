#include <util/elastic_interface.h>

#include "daqontrol/states.h"
#include "global.h"
#include "global_events.h"

namespace Daqontrol {
namespace states {
    void Offline::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Offline");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Unknown");
        global.sendEvent(StateUpdate{});
    }

    void Unknown::react(events::Idle const& e)
    {
        transit<states::Idle>();
    }

    void Unknown::react(events::Configured const& e)
    {
        transit<states::Configured>();
    }

    void Unknown::react(events::Started const& e)
    {
        transit<states::Started>();
    }

    void Idle::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Idle");
        global.sendEvent(StateUpdate{});
    }

    void Idle::react(events::Configured const& e)
    {
        transit<states::Configured>();
    }

    void Configured::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Configured");
        global.sendEvent(StateUpdate{});
    }

    void Configured::react(events::Started const& e)
    {
        transit<states::Started>();
    }

    void Started::entry()
    {
        g_elastic.log(INFO, "Daqontrol : Started");
        global.sendEvent(StateUpdate{});
    }

    void Started::react(events::Configured const& e)
    {
        transit<states::Configured>();
    }
}
}