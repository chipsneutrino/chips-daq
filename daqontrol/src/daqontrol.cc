/**
 * Program name: DAQontrol, by Josh Tingey MSci, JoshTingeyDAQDemon.Josh
 *
 * Author: Josh Tingey
 * E-mail: j.tingey.16@ucl.ac.uk
 */

#include <util/elastic_interface.h>

namespace exit_code {
static constexpr int success = 0;
}

int main(int argc, char* argv[])
{
    // Initialise the elasticsearch interface.
    g_elastic.init(true, false, 10); // log to stdout and use 10 threads for indexing

    g_elastic.log(INFO, "Starting DAQontrol");

    // Contents

    g_elastic.log(INFO, "Stopping DAQontrol");
    return exit_code::success;
}
