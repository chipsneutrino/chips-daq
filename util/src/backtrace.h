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

#include <ostream>

void backtrace(std::ostream& _out) noexcept;