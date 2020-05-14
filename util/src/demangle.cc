#include <cstdlib>
#include <memory>

#include <cxxabi.h>

#include "demangle.h"

namespace {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
std::unique_ptr<char, decltype(std::free)&> demangled_name { nullptr, std::free };
#pragma clang diagnostic pop

}

char const*
get_demangled_name(char const* const symbol) noexcept
{
    if (!symbol) {
        return "<null>";
    }
    int status = -4;
    demangled_name.reset(abi::__cxa_demangle(symbol, demangled_name.release(), nullptr, &status));
    return ((status == 0) ? demangled_name.get() : symbol);
}