#include <sstream>
#include "NssUtil.h"

using namespace nova::NovaSpillServer;

bool NssUtil::gpstimeToString(uint64_t const& gps, std::string& st)
{
  // This function is used to send the gps time through xmlrpc message.
  // The string is not meant to be human readable time.
  std::ostringstream ostr;
  ostr << gps;

  st = ostr.str();

  return true;
}


bool NssUtil::gpstimeFromString(uint64_t& gps, std::string const& st)
{
  //ToDo: check for any errors
  std::istringstream istr(st);
  istr >> gps;

  return true;
}
