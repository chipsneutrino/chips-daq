#include <util/elastic_interface.h>

#include "control_bus/bus_master.h"
#include "control_bus/events.h"
#include "global.h"

namespace ControlBus {
BusMaster::BusMaster(const std::string& bus_url)
    : Publisher { bus_url }
{
    setUnitName("BusMaster");
}

void BusMaster::connected()
{
    log(INFO, "ControlBus publishing to '{}'", bus_url());
    global.sendEvent(events::Connected {});
}

void BusMaster::disconnected(const nng::exception& e)
{
    log(ERROR, "ControlBus caught error: {}: {}", e.who(), e.what());
    global.sendEvent(events::Disconnected {});
}
}
