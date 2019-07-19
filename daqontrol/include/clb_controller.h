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

    /// Methods to post the transition work to the controllers io_service
    void postInit() { io_service_->post(boost::bind(&CLBController::init, this)); };
    void postConfigure() { io_service_->post(boost::bind(&CLBController::configure, this)); };
    void postStartData() { io_service_->post(boost::bind(&CLBController::startData, this)); };
    void postStopData() { io_service_->post(boost::bind(&CLBController::stopData, this)); };

private:
    /// Calls methods to initialise the CLB and updates the state accordingly
    void init();

    /// Calls methods to configure the CLB and updates the state accordingly
    void configure();

    /// Calls methods to start data on the CLB and updates the state accordingly
    void startData();

    /// Calls methods to stop data on the CLB and updates the state accordingly
    void stopData();

    /// Tests the connection to the CLB by sending a message and matching the response eid
    bool testConnection();

    /// Sets server_ip, time_slice_duration, AHRS and disabled acoustic data
    bool setInitValues();    

    /// Resets the CLB from any state to Idle
    bool resetState();

    /// Makes a state change on the CLB ensuring it is a valid transition
    bool setState(CLBEvent event);

    /// Updates the locally stored state from the CLB
    bool getState();

    /// Sets which PMTs are enabled and at what voltage from the configuration
    bool setPMTs();

    /// Checks the PMTs have been set correctly according to the configuration
    bool checkPMTs();

    /// Gets the mask SYS_SYS_RUN_ENA (used in nanobeacon toggling)
    char getSysEnabledMask();

    /// Gets the mask SYS_SYS_DISABLE (used in HV toggling)
    char getSysDisabledMask();

    /// Sets the flasher (on/off) and at what voltage according to the configuration
    bool setFlasher();

    /// Checks the flasher voltage has been set correctly according to the configuration
    bool checkFlasherVoltage();

    /// Enable the HV (NOT CURRENTLY USED)
    bool enableHV();

    /// Disable the HV (NOT CURRENTLY USED)
    bool disableHV();

    /// Gets the IPMux ports (NOT CURRENTLY USED)
    bool getIPMuxPorts();

    MsgProcessor processor_;    ///< Message processor used to communicate with CLB
    CLBState clb_state_;        ///< Current CLB state
};
