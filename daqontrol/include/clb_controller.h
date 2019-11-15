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

#include <tuple>

class CLBController: public Controller {
public:
    /// Create a CLBController, calling Controller constructor and created MsgProcessor
    CLBController(ControllerConfig config, bool disable_hv);

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

    /// Set the enabled channels on the CLB
    bool setEnabledPMTs();

    // Set the HV on the PMTs
    bool setHV();

    // Set the threshold on the PMTs
    bool setThresholds();

    /// Checks the PMT eid values against the configuration
    bool checkIDs();

    /// Checks the enabled channels on the CLB
    bool checkEnabledPMTs();

    // Checks the HVs have been set correctly
    bool checkHV();

    // Checks the threshold on the PMts
    bool checkThresholds();

    /// Gets the mask SYS_SYS_RUN_ENA (used in nanobeacon toggling)
    char getSysEnabledMask();

    /// Gets the mask SYS_SYS_DISABLE (used in HV toggling)
    char getSysDisabledMask();

    /// Sets the flasher (on/off) and at what voltage according to the configuration
    bool setFlasher();

    /// Checks the flasher voltage has been set correctly according to the configuration
    bool checkFlasherVoltage();

    /// Enable the HV
    bool enableHV();

    /// Disable the HV
    bool disableHV();

    /// Sets the IPMux ports
    bool setIPMuxPorts();

    /// Gets the IPMux ports
    bool getIPMuxPorts();

    MsgProcessor processor_;    ///< Message processor used to communicate with CLB
    CLBState clb_state_;        ///< Current CLB state
};
