/**
 * Controller - Base class for a controller, either a CLB or BBB
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <util/daq_config.h>
#include <util/logging.h>

namespace Control {
enum Status {Ready, Configured, Started};
}

class Controller: protected Logging {
public:
    /// Create a Controller using a ControllerConfig
    Controller(ControllerConfig config)
        : config_(config)
        , io_service_{ new boost::asio::io_service }
        , run_work_{ new boost::asio::io_service::work(*io_service_) }
        , thread_( [&]{(*io_service_).run();} )
        , dropped_(false)
        , working_(false)
        , state_(Control::Ready) {};

    /// Destroy a Controller
    virtual ~Controller()
    {
        run_work_.reset();
        io_service_->stop();
    }

    /// Returns the current control state of this controller
    Control::Status getState()
    {
        return state_;
    }

    bool isWorking()
    {
        return working_;
    }

    int getID()
    {
        return config_.eid_;
    }

    void drop()
    {
        dropped_ = true;
    }

    bool dropped()
    {
        return dropped_;
    }

    void setConfig(ControllerConfig config)
    {
        config_ = config;
    }

    virtual void postReset()                        = 0;
    virtual void postConfigure()                    = 0;
    virtual void postStartData()                    = 0;
    virtual void postStopData()                     = 0;

    ControllerConfig config_;                                   ///< Controller specific configuration

protected:
    virtual void reset()                            = 0;
    virtual void configure()                        = 0;
    virtual void startData()                        = 0;
    virtual void stopData()                         = 0;

    std::shared_ptr<boost::asio::io_service> io_service_;       ///< BOOST io_service. The heart of everything
    std::unique_ptr<boost::asio::io_service::work> run_work_;   ///< Work for the io_service
    boost::thread thread_;                                      ///< Thread this controller uses

    bool dropped_;                                              ///< Has this controller been dropped?
    bool working_;                                              ///< Is this controller currently doing work?
    Control::Status state_;                                     ///< Control state of this controller
};
