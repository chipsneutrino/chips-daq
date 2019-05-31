#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <functional>
#include <thread>
#include <mutex>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>
#include <nngpp/protocol/rep0.h>
#include <nngpp/protocol/sub0.h>
#include <tinyfsm.hpp>

#include <util/control_msg.h>
#include <util/elastic_interface.h>

static std::condition_variable cv_terminate{};
static std::mutex mtx_terminate{};
static std::atomic_bool running{true};

static std::recursive_mutex mtx_dispatch{};

template <typename E>
void send_event(E const &event);

struct KillSignal : tinyfsm::Event
{
};
struct StateUpdate : tinyfsm::Event
{
};

namespace Ops
{
namespace events
{
struct StartRun;
struct StopRun;
} // namespace events
} // namespace Ops

namespace CHIPS
{
class FSM : public tinyfsm::Fsm<FSM>
{
public:
    virtual void entry(void) = 0;
    void exit(void) {}

    void react(KillSignal const &);
    virtual void react(StateUpdate const &update) {}
    virtual void react(Ops::events::StartRun const &) {}
    void react(tinyfsm::Event const &) {}
};

namespace states
{
class Init : public FSM
{
    void entry() override;
    void react(StateUpdate const &) override;
};
class Exit : public FSM
{
    void entry() override;
};
class Ready : public FSM
{
    void entry() override;
    void react(Ops::events::StartRun const &) override;
};
class StartingRun : public FSM
{
    void entry() override;
    void react(StateUpdate const &) override;
};
class Run : public FSM
{
    void entry() override;
};
class StoppingRun : public FSM
{
    void entry() override;
};
class Error : public FSM
{
    void entry() override;
};
} // namespace states
} // namespace CHIPS
////////

namespace ControlBus
{
namespace events
{
struct Disconnected : tinyfsm::Event
{
};
struct Connected : tinyfsm::Event
{
};
} // namespace events

class FSM : public tinyfsm::Fsm<FSM>
{
public:
    virtual void entry(void) = 0;
    void exit(void) {}

    void react(tinyfsm::Event const &) {}
    virtual void react(events::Disconnected const &) {}
    virtual void react(events::Connected const &) {}
};

namespace states
{
class Offline : public FSM
{
    void entry() override;
    void react(events::Connected const &e);
};

class Online : public FSM
{
    void entry() override;
    void react(events::Disconnected const &e) override;
};
} // namespace states

struct Manager
{
    void run();
    void runAsync();

    using message_type = ControlMessage;

    std::list<message_type> publish_queue_;
    std::condition_variable cv_publish_queue_;
    std::mutex mtx_publish_queue_;
    std::chrono::milliseconds reconnect_interval_{5000};

    void publish(message_type &&message);
};
} // namespace ControlBus

////////

namespace Daqonite
{
namespace events
{
struct Disconnected : tinyfsm::Event
{
};
struct Connected : tinyfsm::Event
{
};
struct Idle : tinyfsm::Event
{
};
struct RunInProgress : tinyfsm::Event
{
};
} // namespace events

class FSM : public tinyfsm::Fsm<FSM>
{
public:
    virtual void entry(void) = 0;
    void exit(void) {}

    void react(tinyfsm::Event const &) {}
    virtual void react(events::Disconnected const &);
    virtual void react(events::Connected const &);
    virtual void react(events::Idle const &) {}
    virtual void react(events::RunInProgress const &) {}
};

namespace states
{
class Offline : public FSM
{
    void entry() override;
    void react(events::Disconnected const &) override {}
};

class Unknown : public FSM
{
    void entry() override;
    void react(events::Idle const &) override;
    void react(events::RunInProgress const &) override;
    void react(events::Connected const &) override {}
};

class Idle : public FSM
{
    void entry() override;
    void react(events::RunInProgress const &) override;
};

class RunInProgress : public FSM
{
    void entry() override;
    void react(events::Idle const &) override;
};
} // namespace states

struct Manager
{
    void run();
    void runAsync();
};
} // namespace Daqonite

////////

namespace Ops
{
namespace events
{
struct StartRun : tinyfsm::Event
{
};
struct StopRun : tinyfsm::Event
{
};
} // namespace events

struct Manager
{
    void run();
    void runAsync();
};
} // namespace Ops

////////

static ControlBus::Manager ct{};
static Daqonite::Manager daqonite{};
static Ops::Manager ops{};

////////

namespace CHIPS
{
void FSM::react(KillSignal const &e)
{
    transit<states::Exit>();
}

namespace states
{
void Init::entry()
{
    g_elastic.log(INFO, "CHIPS : Init");
    send_event(StateUpdate{});
}

void Init::react(StateUpdate const &)
{
    if (ControlBus::FSM::is_in_state<ControlBus::states::Online>() && Daqonite::FSM::is_in_state<Daqonite::states::Idle>())
    {
        transit<states::Ready>();
    }
}

void Exit::entry()
{
    g_elastic.log(INFO, "CHIPS : Exit");
    send_event(StateUpdate{});

    std::lock_guard<std::mutex> lk{mtx_terminate};
    running = false;
    cv_terminate.notify_all();
}

void Ready::entry()
{
    g_elastic.log(INFO, "CHIPS : Ready");
    send_event(StateUpdate{});
}

void Ready::react(Ops::events::StartRun const &e)
{
    transit<states::StartingRun>();
}

void StartingRun::entry()
{
    g_elastic.log(INFO, "CHIPS : StartingRun");
    send_event(StateUpdate{});

    {
        ControlMessage msg{};
        msg.Discriminator = ControlMessage::StartRun::Discriminator;
        msg.Payload.pStartRun = ControlMessage::StartRun{};
        msg.Payload.pStartRun.Which = RunType::TestDAQ; // FIXME: parametric
        ct.publish(std::move(msg));
    }
}

void StartingRun::react(StateUpdate const &)
{
    if (!ControlBus::FSM::is_in_state<ControlBus::states::Online>())
    {
        transit<states::Error>();
        return;
    }

    if (!Daqonite::FSM::is_in_state<Daqonite::states::Idle>() && !Daqonite::FSM::is_in_state<Daqonite::states::RunInProgress>())
    {
        transit<states::Error>();
        return;
    }

    if (Daqonite::FSM::is_in_state<Daqonite::states::RunInProgress>())
    {
        transit<states::Run>();
        return;
    }
}

void Run::entry()
{
    g_elastic.log(INFO, "CHIPS : Run");
    send_event(StateUpdate{});
}

void StoppingRun::entry()
{
    g_elastic.log(INFO, "CHIPS : StoppingRun");
    send_event(StateUpdate{});
}

void Error::entry()
{
    g_elastic.log(INFO, "CHIPS : Error");
    send_event(StateUpdate{});
}
} // namespace states
} // namespace CHIPS

FSM_INITIAL_STATE(CHIPS::FSM, CHIPS::states::Init)

////////

namespace ControlBus
{
void Manager::run()
{
    for (;;)
    {
        try
        {
            auto sock = nng::pub::open();
            sock.listen(message_type::URL);
            g_elastic.log(INFO, "ControlBus publishing to '{}'", message_type::URL);

            send_event(events::Connected{});

            for (;;)
            {
                std::unique_lock<std::mutex> lk{mtx_publish_queue_};

                while (!publish_queue_.empty())
                {
                    g_elastic.log(INFO, "ControlBus sending");
                    sock.send(nng::view{&publish_queue_.front(), sizeof(message_type)});
                    publish_queue_.pop_front();
                }

                g_elastic.log(INFO, "ControlBus waiting for message");
                cv_publish_queue_.wait(lk, [this] { return !publish_queue_.empty(); });
            }
        }
        catch (const nng::exception &e)
        {
            g_elastic.log(ERROR, "ControlBus caught error: {}: {}", e.who(), e.what());
            send_event(events::Disconnected{});
            std::this_thread::sleep_for(reconnect_interval_);
        }
    }
}

void Manager::runAsync()
{
    std::thread{std::bind(&Manager::run, this)}.detach();
}

void Manager::publish(message_type &&message)
{
    std::lock_guard<std::mutex> lk{mtx_publish_queue_};
    publish_queue_.emplace_back(std::move(message));
    cv_publish_queue_.notify_one();
}

namespace states
{
void Offline::entry()
{
    g_elastic.log(INFO, "ControlBus : Offline");
    send_event(StateUpdate{});
}

void Offline::react(events::Connected const &e)
{
    transit<states::Online>();
}

void Online::entry()
{
    g_elastic.log(INFO, "ControlBus : Online");
    send_event(StateUpdate{});
}

void Online::react(events::Disconnected const &e)
{
    transit<states::Offline>();
}
} // namespace states
} // namespace ControlBus

FSM_INITIAL_STATE(ControlBus::FSM, ControlBus::states::Offline)

////////

namespace Daqonite
{
void Manager::run()
{
    for (;;)
    {
        try
        {
            auto sock = nng::sub::open();
            nng::sub::set_opt_subscribe(sock, "");
            nng::set_opt_recv_timeout(sock, 1000);
            sock.dial(DaqoniteStateMessage::URL);

            send_event(events::Connected{});

            DaqoniteStateMessage message{};
            for (;;)
            {
                sock.recv(nng::view{&message, sizeof(message)});

                if (message.RunMode)
                {
                    send_event(events::RunInProgress{});
                }
                else
                {
                    send_event(events::Idle{});
                }
            }
        }
        catch (const nng::exception &e)
        {
            g_elastic.log(DEBUG, "Daqonite error: {}: {}", e.who(), e.what());
            send_event(events::Disconnected{});
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void Manager::runAsync()
{
    std::thread{std::bind(&Manager::run, this)}.detach();
}

void FSM::react(events::Disconnected const &e)
{
    transit<states::Offline>();
}

void FSM::react(events::Connected const &e)
{
    transit<states::Unknown>();
}

namespace states
{
void Offline::entry()
{
    g_elastic.log(INFO, "Daqonite : Offline");
    send_event(StateUpdate{});
}

void Unknown::entry()
{
    g_elastic.log(INFO, "Daqonite : Unknown");
    send_event(StateUpdate{});
}

void Unknown::react(events::Idle const &e)
{
    transit<states::Idle>();
}

void Unknown::react(events::RunInProgress const &e)
{
    transit<states::RunInProgress>();
}

void Idle::entry()
{
    g_elastic.log(INFO, "Daqonite : Idle");
    send_event(StateUpdate{});
}

void Idle::react(events::RunInProgress const &e)
{
    transit<states::RunInProgress>();
}

void RunInProgress::entry()
{
    g_elastic.log(INFO, "Daqonite : RunInProgress");
    send_event(StateUpdate{});
}

void RunInProgress::react(events::Idle const &e)
{
    transit<states::Idle>();
}
} // namespace states
} // namespace Daqonite

FSM_INITIAL_STATE(Daqonite::FSM, Daqonite::states::Offline)

////////

namespace Ops
{
void Manager::run()
{
    for (;;)
    {
        try
        {
            auto sock = nng::rep::open();
            sock.listen(OpsMessage::URL);

            OpsMessage message{};

            const bool t = true;
            const bool f = false;
            const nng::view ack{&t, sizeof(t)};
            const nng::view nak{&f, sizeof(f)};

            for (;;)
            {
                sock.recv(nng::view{&message, sizeof(message)});

                switch (message.Discriminator)
                {
                case OpsMessage::StartRun::Discriminator:
                    send_event(events::StartRun{});
                    if (CHIPS::FSM::is_in_state<CHIPS::states::StartingRun>())
                    {
                        sock.send(ack);
                    }
                    else
                    {
                        sock.send(nak);
                    }

                    break;

                case OpsMessage::StopRun::Discriminator:
                    send_event(events::StopRun{});
                    sock.send(ack);
                    break;

                default:
                    sock.send(nak);
                    break;
                }
            }
        }
        catch (const nng::exception &e)
        {
            g_elastic.log(DEBUG, "Ops error: {}: {}", e.who(), e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void Manager::runAsync()
{
    std::thread{std::bind(&Manager::run, this)}.detach();
}
} // namespace Ops

////////

using fsm_type = tinyfsm::FsmList<CHIPS::FSM, ControlBus::FSM, Daqonite::FSM>;
template <typename E>
void send_event(E const &event)
{
    std::lock_guard<std::recursive_mutex> lk{mtx_dispatch};
    fsm_type::template dispatch<E>(event);
}

void signal_handler(int signal)
{
    switch (signal)
    {
    case SIGINT:
        g_elastic.log(INFO, "Received signal {}. Terminating...", signal);
        send_event(KillSignal{});
        break;
    default:
        g_elastic.log(WARNING, "Caught unhandled signal {}.", signal);
        break;
    }
}

int main(int argc, char *argv[])
{
    g_elastic.init(true, false, 10); // log to stdout and use 10 threads for indexing
    g_elastic.log(INFO, "CHIPS FSM started");

    std::signal(SIGINT, signal_handler);

    fsm_type::start();

    ct.runAsync();
    daqonite.runAsync();
    ops.runAsync();

    {
        std::unique_lock<std::mutex> lk{mtx_terminate};
        cv_terminate.wait(lk, [] { return !running; });
    }

    g_elastic.log(INFO, "CHIPS FSM finished");
    return 0;
}
