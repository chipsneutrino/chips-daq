#include <util/elastic_interface.h>

#include "daqsitter/states.h"
#include "global.h"
#include "global_events.h"

namespace Daqsitter {
namespace states {
    void Offline::entry()
    {
        log(INFO, "Daqsitter : Offline");
        global.sendEvent(StateUpdate {});
    }

    void Unknown::entry()
    {
        log(INFO, "Daqsitter : Unknown");
        global.sendEvent(StateUpdate {});
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
        log(INFO, "Daqsitter : Ready");
        global.sendEvent(StateUpdate {});
    }

    void Ready::react(events::Started const& e)
    {
        transit<states::Started>();
    }

    void Started::entry()
    {
        log(INFO, "Daqsitter : Started");
        global.sendEvent(StateUpdate {});
    }

    void Started::react(events::Ready const& e)
    {
        transit<states::Ready>();
    }
}
}