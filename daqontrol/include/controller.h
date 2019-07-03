/**
 * Controller - Controller for an individual CLB
 */

#pragma once

#include "msg_processor.h"
#include "msg_types.h"
#include "proc_var.h"
#include "clb_subsys.h"
#include "clb_event.h"

#include "util/daq_config.h"
#include <util/elastic_interface.h>

class Controller {
public:
    /**
     * Create a Controller
     * This creates a Controller, setting up the MsgProcessor etc...
     */
    Controller(ControllerConfig config);

    /// Destroy a Controller
    ~Controller();

    void postTest();
    void postInit();
    void postConfigure();
    void postStartRun();
    void postStopRun();

    void setInitValues();    
    void addNanobeacon(std::vector<int> &vid, std::vector<long> &vv);
    void disableNanobeacon();

    void disableHV();

    void clbEvent(int event_id);
    void setPMTs();
    void checkPMTs();

    void askPMTsInfo(int info_type);
    void askVars(std::vector<int> var_ids);
    void askState();

private:
    /**
     * Bound to thread creation
     * Allows us to modify how the thread operates and what it does
     */
    void ioServiceThread();

    void test();
    void init();
    void configure();
    void startRun();
    void stopRun();

    ControllerConfig config_;                                   ///< Controller specific configuration

    std::shared_ptr<boost::asio::io_service> io_service_;       ///< BOOST io_service. The heart of everything
    std::unique_ptr<boost::asio::io_service::work> run_work_;   ///< Work for the io_service
    boost::thread thread_;                                      ///< Thread this controller uses

    MsgProcessor processor_;                                    ///< Message processor used to communicate with CLB
};
