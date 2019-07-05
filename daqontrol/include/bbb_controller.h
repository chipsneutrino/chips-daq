/**
 * BBBController - BBBController for an individual CLB
 */

#pragma once

#include "controller.h"
#include <util/elastic_interface.h>

class BBBController: public Controller {
public:
    /// Create a BBBController, calling Controller constructor and created MsgProcessor
    BBBController(ControllerConfig config);

    /// Destroy a BBBController
    ~BBBController() {};

    void postInit();
    void postConfigure();
    void postStart();
    void postStop();

private:
    void init();
    void configure();
    void start();
    void stop();
};
