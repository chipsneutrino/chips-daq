#ifndef __NSS_UTIL_H_
#define __NSS_UTIL_H_

#include <string>
#include <stdint.h>  /* uint64_t */

namespace nova {
namespace NovaSpillServer {

//namespace NssConst {
//  // Jan.6 1980 0000 UT to Jan.1 1970 0000 UT
//  // 10 years (two leap years) + 5 days + 9 leap seconds
//  int const kGpsToUnixEpocheOffset = - 24*3600*(10*365+2+5) +9;
//  
//  int const kNsPerS = 1000000000;
//}

class NssUtil {
 public:

  static bool gpstimeToString(uint64_t const& gps, std::string& st);
  static bool gpstimeFromString(uint64_t& gps, std::string const& st);

};

} // end of namespace NovaSpillServer
} // end of namespace nova

#endif //__NSS_UTIL_H_
