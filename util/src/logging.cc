#include "logging.h"
#include "elastic_interface.h"

Logging::Logging()
    : unit_name_ { "unknown_unit" }
{
}

void Logging::log(severity level, std::string&& message)
{
    g_elastic.log(level, message);
}