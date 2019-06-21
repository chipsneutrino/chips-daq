/**
 * Controller - Controller for an individual CLB
 */

#pragma once

#include "msg_processor.h"
#include "msg_types.h"
#include "proc_var.h"

#include <util/elastic_interface.h>

class Controller {
public:
    /**
     * Create a Controller
     * This creates a Controller, setting up the MsgProcessor etc...
     */
    Controller(unsigned long ip_address);

    /// Destroy a Controller
    ~Controller();

    void postDaterev();
    void postGetVars(std::vector<int> varIds);
    void postSetVars(std::map<int, int> toModify);

private:
    /**
     * Bound to thread creation
     * Allows us to modify how the thread operates and what it does
     */
    void ioServiceThread();

    void daterev();
    void getVars(std::vector<int> varIds);
    void setVars(std::map<int, int> toModify);

    std::shared_ptr<boost::asio::io_service> io_service_;       ///< BOOST io_service. The heart of everything
    std::unique_ptr<boost::asio::io_service::work> run_work_;   ///< Work for the io_service
    boost::thread thread_;                                      ///< Thread this controller uses

    MsgProcessor processor_;                                    ///< Message processor used to communicate with CLB
};
