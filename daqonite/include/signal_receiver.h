#include <atomic>
#include <memory>

#include <boost/asio.hpp>

#include "command_receiver.h"

class SignalReceiver {
public:
    explicit SignalReceiver();
    virtual ~SignalReceiver() = default;

    void setHandler(std::shared_ptr<CommandHandler> handler);

    void runAsync();
    void join();

private:
    std::shared_ptr<CommandHandler> handler_;

    std::atomic_bool running_;
    std::unique_ptr<std::thread> receiver_thread_;

    boost::asio::io_service io_service_;
    boost::asio::signal_set signal_set_; ///< BOOST signal_set

    void receiverThread();

    /**
     * Handles UNIX signals
     * Handles the interupts from UNIX signals. Currently only ctrl-c is defined
     * which calls exit() on the application.
     * 
     * @param error Signals error code
     * @param signum Signal number
     */
    void handleSignals(boost::system::error_code const& error, int signum);

    /// Calls the async_wait() on the signal_set
    void workSignals();
};
