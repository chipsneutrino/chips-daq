#include <util/elastic_interface.h>

#include "daqontrol/states.h"
#include "global.h"
#include "global_events.h"

namespace Daqontrol {
namespace states {
    void Offline::entry()
    {
        log(INFO, "Daqontrol : Offline");
        global.sendEvent(StateUpdate {});
    }

    void Unknown::entry()
    {
        log(INFO, "Daqontrol : Unknown");
        global.sendEvent(StateUpdate {});
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
        log(INFO, "Daqontrol : Initialising");
        global.sendEvent(StateUpdate {});
    }

    void Initialising::react(events::Ready const& e)
    {
        transit<states::Ready>();
    }

    void Ready::entry()
    {
        log(INFO, "Daqontrol : Ready");
        global.sendEvent(StateUpdate {});
    }

    void Ready::react(events::Configured const& e)
    {
        transit<states::Configured>();
    }

    void Configured::entry()
    {
        log(INFO, "Daqontrol : Configured");
        global.sendEvent(StateUpdate {});
    }

    void Configured::react(events::Started const& e)
    {
        transit<states::Started>();
    }

    void Started::entry()
    {
        log(INFO, "Daqontrol : Started");
        global.sendEvent(StateUpdate {});
    }

    void Started::react(events::Configured const& e)
    {
        transit<states::Configured>();
    }
}
}