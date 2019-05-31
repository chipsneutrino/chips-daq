#include <util/elastic_interface.h>

#include "control_bus/states.h"
#include "global.h"
#include "global_events.h"

namespace ControlBus {
namespace states {
    void Offline::entry()
    {
        g_elastic.log(INFO, "ControlBus : Offline");
        global.sendEvent(StateUpdate{});
    }

    void Offline::react(events::Connected const& e)
    {
        transit<states::Online>();
    }

    void Online::entry()
    {
        g_elastic.log(INFO, "ControlBus : Online");
        global.sendEvent(StateUpdate{});
    }

    void Online::react(events::Disconnected const& e)
    {
        transit<states::Offline>();
    }
}
}