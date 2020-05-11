#include "logging.h"
#include "elastic_interface.h"

Logging::Logging()
    : unit_name_ { "unknown_unit" }
{
}

void Logging::setUnitName(std::string&& unit_name)
{
    unit_name_ = unit_name;
}

void Logging::log(severity level, std::string&& message)
{
    g_elastic.log(level, unit_name_, std::move(message));
}
