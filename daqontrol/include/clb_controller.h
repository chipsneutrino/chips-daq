/**
 * CLBController - CLBController for an individual CLB
 */

#pragma once

#include <clb/msg_processor.h>
#include <clb/msg_types.h>
#include <clb/proc_var.h>
#include <clb/var_info.h>
#include <clb/clb_subsys.h>
#include <clb/clb_event.h>

#include "controller.h"
#include <util/elastic_interface.h>

class CLBController: public Controller {
public:
    /// Create a CLBController, calling Controller constructor and created MsgProcessor
    CLBController(ControllerConfig config);

    /// Destroy a CLBController
    ~CLBController() {};

    void postInit() { io_service_->post(boost::bind(&CLBController::init, this)); };
    void postConfigure() { io_service_->post(boost::bind(&CLBController::configure, this)); };
    void postStartData() { io_service_->post(boost::bind(&CLBController::startData, this)); };
    void postStopData() { io_service_->post(boost::bind(&CLBController::stopData, this)); };

private:
    void init();
    void configure();
    void startData();
    void stopData();

    bool testConnection();
    bool setInitValues();    

    bool resetState();
    bool setState(CLBEvent event);
    bool getState();

    bool setPMTs();
    bool checkPMTs();

    char getSysEnabledMask();
    char getSysDisabledMask();

    bool setFlasher();
    bool checkFlasherVoltage();

    bool enableHV();
    bool disableHV();

    bool getIPMuxPorts();

    MsgProcessor processor_;    ///< Message processor used to communicate with CLB
    CLBState clb_state_;        ///< Current CLB state
};
