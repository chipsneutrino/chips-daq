/**
 * Termination handler that attempts to unwind stack and annotate any active
 * exceptions. The results are first printed to the output but then the best effort
 * is made to send a last breath message to ElasticSearch.
 * 
 * The following implementation is inspired by Eli Bendersky & Anatoliy Tomilov.
 * For more information see:
 * 
 *   https://eli.thegreenplace.net/2015/programmatic-access-to-the-call-stack-in-c/
 * 
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <type_traits>
#include <typeinfo>

#include <cxxabi.h>

#include "backtrace.h"
#include "demangle.h"
#include "elastic_interface.h"

namespace {

[[noreturn]] void
backtrace_on_terminate() noexcept;

static_assert(std::is_same<std::terminate_handler, decltype(&backtrace_on_terminate)> {});

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
std::unique_ptr<std::remove_pointer<std::terminate_handler>::type, decltype(std::set_terminate)&> terminate_handler { std::set_terminate(backtrace_on_terminate), std::set_terminate };
#pragma clang diagnostic pop

[[noreturn]] void
backtrace_on_terminate() noexcept
{
    std::set_terminate(terminate_handler.release()); // to avoid infinite looping if any

    // Print backtrace immediately in case there is something fishy going on
    std::clog << "LOG (FATAL, Main): Terminate called with the following stack:" << std::endl;
    backtrace(std::clog);

    // Attempt to reprint the same message into the ElasticInterface
    std::stringstream ss {};
    ss << "Terminate called with the following stack:" << std::endl;
    backtrace(ss);
    g_elastic.log(FATAL, "Main", ss.str());

    if (std::exception_ptr ep = std::current_exception()) {
        try {
            std::rethrow_exception(ep);
        } catch (std::exception const& e) {
            std::clog << "LOG (FATAL, Main): Unhandled exception std::exception:what(): " << e.what() << std::endl;
            g_elastic.log(FATAL, "Main", "Unhandled exception std::exception:what(): " + std::string { e.what() });
        } catch (...) {
            if (std::type_info* et = abi::__cxa_current_exception_type()) {
                std::clog << "LOG (FATAL, Main): Unhandled exception type: " << get_demangled_name(et->name()) << std::endl;
                g_elastic.log(FATAL, "Main", "LOG (FATAL, Main): Unhandled exception type: " + std::string { get_demangled_name(et->name()) });
            } else {
                std::clog << "LOG (FATAL, Main): Unhandled unknown exception" << std::endl;
                g_elastic.log(FATAL, "Main", "Unhandled unknown exception");
            }
        }
    }

    // Give the ElasticInterface an opportunity to index the log messages.
    g_elastic.stop_and_join();

    std::_Exit(EXIT_FAILURE);
}

}