/**
 * BBBController - BBBController for an individual BBB
 */

#include <chrono>
#include <fstream>

#include "bbb_controller.h"

BBBController::BBBController(ControllerConfig config)
    : Controller { config }
    , control_ { new BadgerboardControl }
    , heartbeat_ { new BadgerboardHeartbeat }
{
    setUnitName("BBBController[{}]", config.eid_);
    log(INFO, "Creating");

    heartbeat_->runAsync();
}

BBBController::~BBBController()
{
    // TODO: implement me
    heartbeat_->notifyJoin();
    heartbeat_->join();
}

void BBBController::reset()
{
    // TODO: implement me
}

void BBBController::configure()
{
    using namespace std::chrono;

    log(DEBUG, "Start configure...");
    working_ = true;

    // TODO: handle errors
    control_->resetConfiguration();

    // FIXME: get this from central config source
    std::vector<char> hub_cfg {};
    loadFileContents(hub_cfg, "/chips/config/test_madison_hub.cfg");

    // TODO: handle errors
    control_->configureHub(hub_cfg.data(), hub_cfg.size());
    heartbeat_->waitForStateChange(milliseconds { 5000 }, [](const BadgerboardState& state) {
        return state.hubConfig == BadgerboardConfigState::Configured;
    });

    // FIXME: get this from central config source
    std::vector<char> run_cfg {};
    loadFileContents(run_cfg, "/chips/config/test_madison_run.cfg");

    // TODO: handle errors
    control_->configureRun(run_cfg.data(), run_cfg.size());
    heartbeat_->waitForStateChange(milliseconds { 5000 }, [](const BadgerboardState& state) {
        return state.runConfig == BadgerboardConfigState::Configured;
    });

    // FIXME: get this from central config source
    BadgerboardChannelSelection desired_power_state {};
    desired_power_state[14] = true;

    // TODO: handle errors
    control_->setPowerState(desired_power_state);
    heartbeat_->waitForStateChange(milliseconds { 45000 }, [&desired_power_state](const BadgerboardState& state) {
        for (std::size_t channel_idx = 0; channel_idx < N_BADGERBOARD_CHANNELS; ++channel_idx) {
            const auto expected_state { desired_power_state[channel_idx]
                    ? BadgerboardChannelState::Started
                    : BadgerboardChannelState::PoweredOff };

            if (state.channels[channel_idx] != expected_state) {
                return false;
            }
        }
        return true;
    });

    state_ = Control::Configured; // Set the controller state to Configured
    log(DEBUG, "Configure DONE");
    working_ = false;
}

void BBBController::startData()
{
    log(DEBUG, "Start Data...");
    working_ = true;

    control_->beginDataRun();
    // TODO: handle errors

    state_ = Control::Started; // Set the controller state to Started
    log(DEBUG, "Start Data DONE");
    working_ = false;
}

void BBBController::stopData()
{
    log(DEBUG, "Stop Data...");
    working_ = true;

    // TODO: implement me

    state_ = Control::Configured; // Set the controller state to Configured
    log(DEBUG, "Stop Data DONE");
    working_ = false;
}

bool BBBController::loadFileContents(std::vector<char>& dest_buffer, const char* file_path)
{
    std::ifstream file { file_path, std::ios::binary | std::ios::ate };
    const std::streamsize file_size { file.tellg() };
    file.seekg(0, std::ios::beg);

    dest_buffer.resize(file_size);
    file.read(dest_buffer.data(), file_size);
    return file.good();
}