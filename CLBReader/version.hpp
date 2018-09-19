#ifndef CLB_READER_VERSION_HPP
#define CLB_READER_VERSION_HPP

namespace clbreader {
namespace version {

const static char major[]   = "1";
const static char minor[]   = "1";
#ifdef SVN_REVISION
const static char svn_rev[] = SVN_REVISION;
#else
const static char svn_rev[] = "undef";
#endif

#ifdef BUILD_DATE
const static char build_date[] = BUILD_DATE;
#else
const static char build_date[] = "undef";
#endif

const static std::string separator("-");

std::string v()
{
  return major
    + std::string(".")
    + minor
    + separator
    + svn_rev
    + separator
    + build_date;
}

} // ns version
} // ns clbreader

#endif // CLB_READER_VERSION_HPP
