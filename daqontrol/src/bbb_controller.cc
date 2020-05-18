/**
 * BBBController - BBBController for an individual BBB
 */

#include <fstream>

#include "bbb_controller.h"

BBBController::BBBController(ControllerConfig config)
    : Controller { config }
    , badgerboard_ { new Badgerboard }
{
    setUnitName("BBBController[{}]", config.eid_);
    log(INFO, "Creating");
}

BBBController::~BBBController()
{
    // TODO: implement me
}

void BBBController::reset()
{
    // TODO: implement me
}

void BBBController::configure()
{
    log(DEBUG, "Start configure...");
    working_ = true;

    // FIXME: get this from central config source
    std::vector<char> hub_cfg {};
    loadFileContents(hub_cfg, "/chips/config/test_madison_hub.cfg");
    badgerboard_->configureHub(hub_cfg.data(), hub_cfg.size());
    // TODO: handle errors

    // TODO: wait until heartbeat shows hub-configured state
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // FIXME: get this from central config source
    std::vector<char> run_cfg {};
    loadFileContents(run_cfg, "/chips/config/test_madison_run.cfg");
    badgerboard_->configureRun(run_cfg.data(), run_cfg.size());
    // TODO: handle errors

    // TODO: wait until heartbeat shows run-configured state
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // TODO: set channel power state

    state_ = Control::Configured; // Set the controller state to Configured
    log(DEBUG, "Configure DONE");
    working_ = false;
}

void BBBController::startData()
{
    log(DEBUG, "Start Data...");
    working_ = true;

    badgerboard_->beginDataRun();
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
