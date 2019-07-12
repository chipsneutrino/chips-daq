/**
 * BBBController - BBBController for an individual CLB
 */

#pragma once

#include "controller.h"
#include "fh_library.h"
#include <util/elastic_interface.h>

class BBBController: public Controller {
public:
    /// Create a BBBController, calling Controller constructor and created MsgProcessor
    BBBController(ControllerConfig config);

    /// Destroy a BBBController
    ~BBBController() {};

    void postInit();
    void postConfigure();
    void postStartData();
    void postStopData();

private:
    void init();
    void configure();
    void startData();
    void stopData();
};
