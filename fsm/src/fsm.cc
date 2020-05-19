#include <csignal>
#include <iostream>

#include <util/config.h>
#include <util/elastic_interface.h>
#include <util/singleton_process.h>

#include "fsm.h"
#include "global.h"
#include "global_events.h"

void signal_handler(int signal)
{
    // For nice output.
    std::cerr << std::endl;

    switch (signal) {
    case SIGINT:
        // FIXME: log(INFO, "Received signal {}. Terminating...", signal);
        global.sendEvent(KillSignal {});
        break;
    default:
        // FIXME: log(WARNING, "Caught unhandled signal {}.", signal);
        break;
    }
}

class Main : protected Logging {
public:
    Main()
        : Logging {}
    {
        setUnitName("Main");
    }

    int main(int argc, char* argv[])
    {
        // Check that no other FSM instances are running
        SingletonProcess singleton(11110);
        if (!singleton())
            throw std::runtime_error("FSM already running!");

        const std::string process_name { argv[0] };

        g_config.init(process_name);
        g_elastic.init(process_name);
        log(INFO, "FSM started");

        std::signal(SIGINT, signal_handler);

        global.readSettings(argc, argv);
        global.setupComponents();

        MainFSM::start();

        global.runComponents();
        global.waitUntilTerminated();
        global.notifyAndJoinComponents();

        log(INFO, "FSM finished");
        return 0;
    }
};

int main(int argc, char* argv[])
{
    Main app {};
    return app.main(argc, argv);
}
