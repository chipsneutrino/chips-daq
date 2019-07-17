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
    ~CLBController();

    void postInit() { io_service_->post(boost::bind(&CLBController::init, this)); };
    void postConfigure() { io_service_->post(boost::bind(&CLBController::configure, this)); };
    void postStartData() { io_service_->post(boost::bind(&CLBController::startData, this)); };
    void postStopData() { io_service_->post(boost::bind(&CLBController::stopData, this)); };
    void postEnableFlasher(float flasher_v) { io_service_->post(boost::bind(&CLBController::enableFlasher, this, flasher_v)); };
    void postDisableFlasher() { io_service_->post(boost::bind(&CLBController::disableFlasher, this)); }; 

private:
    void init();
    void configure();
    void startData();
    void stopData();

    void testConnection();
    void setInitValues();    

    void resetState();
    void setState(CLBEvent event);
    void getState();

    void setPMTs();
    void checkPMTs();

    char getSysEnabledMask();
    char getSysDisabledMask();
    void getMasks();

    void enableFlasher(float flasher_v);
    void checkFlasherVoltage(float flasher_v);
    void disableFlasher();
    void enableHV();

    void disableHV();

    void getIPMuxPorts();

    MsgProcessor processor_;    ///< Message processor used to communicate with CLB
    CLBState state_;            ///< Current CLB state
};
