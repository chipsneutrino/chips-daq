/**
 * Controller - Base class for a controller, either a CLB or BBB
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <util/daq_config.h>

class Controller {
public:
    /// Create a Controller using a ControllerConfig
    Controller(ControllerConfig config)
        : config_(config)
        , io_service_{ new boost::asio::io_service }
        , run_work_{ new boost::asio::io_service::work(*io_service_) }
        , thread_( [&]{(*io_service_).run();} ) {};

    /// Destroy a Controller
    virtual ~Controller()
    {
        run_work_.reset();
        io_service_->stop();
    }

    virtual void postInit() = 0;
    virtual void postConfigure() = 0;
    virtual void postStartData() = 0;
    virtual void postStopData() = 0;
    virtual void postFlasherOn(float flasher_v) = 0;
    virtual void postFlasherOff() = 0;

protected:
    virtual void init() = 0;
    virtual void configure() = 0;
    virtual void startData() = 0;
    virtual void stopData() = 0;
    virtual void flasherOn(float flasher_v) = 0;
    virtual void flasherOff() = 0;

    ControllerConfig config_;                                   ///< Controller specific configuration

    std::shared_ptr<boost::asio::io_service> io_service_;       ///< BOOST io_service. The heart of everything
    std::unique_ptr<boost::asio::io_service::work> run_work_;   ///< Work for the io_service
    boost::thread thread_;                                      ///< Thread this controller uses
};
