#include <util/elastic_interface.h>

#include "control_bus/bus_master.h"
#include "control_bus/events.h"
#include "global.h"

namespace ControlBus {
void BusMaster::connected()
{
    g_elastic.log(INFO, "ControlBus publishing to '{}'", message_type::URL);
    global.sendEvent(events::Connected{});
}

void BusMaster::disconnected(const nng::exception& e)
{
    g_elastic.log(ERROR, "ControlBus caught error: {}: {}", e.who(), e.what());
    global.sendEvent(events::Disconnected{});
}
}