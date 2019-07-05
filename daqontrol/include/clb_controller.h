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

    void postInit();
    void postConfigure();
    void postStart();
    void postStop();

private:
    void init();
    void configure();
    void start();
    void stop();

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

    void test();
    void quit();
    void reset();
    void pause();
    void continueRun();

    MsgProcessor processor_; ///< Message processor used to communicate with CLB
};
