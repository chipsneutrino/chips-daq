#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <ostream>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "backtrace.h"
#include "demangle.h"

namespace {

void print_reg(std::ostream& _out, unw_word_t reg) noexcept
{
    constexpr std::size_t address_width = std::numeric_limits<std::uintptr_t>::digits / 4;
    _out << "0x" << std::setfill('0') << std::setw(address_width) << reg;
}

char symbol[1024];

}

void backtrace(std::ostream& _out) noexcept
{
    unw_cursor_t cursor;
    unw_context_t context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    _out << std::hex << std::uppercase;
    while (0 < unw_step(&cursor)) {
        unw_word_t ip = 0;
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        if (ip == 0) {
            break;
        }
        unw_word_t sp = 0;
        unw_get_reg(&cursor, UNW_REG_SP, &sp);
        print_reg(_out, ip);
        _out << ": (SP:";
        print_reg(_out, sp);
        _out << ") ";
        unw_word_t offset = 0;
        if (unw_get_proc_name(&cursor, symbol, sizeof(symbol), &offset) == 0) {
            _out << "(" << get_demangled_name(symbol) << " + 0x" << offset << ")" << std::endl;
        } else {
            _out << "-- error: unable to obtain symbol name for this frame" << std::endl;
        }
    }
    _out << std::flush;
}