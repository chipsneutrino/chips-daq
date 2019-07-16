/**
 * CLBController - CLBController for an individual CLB
 */

#include "clb_controller.h"

CLBController::CLBController(ControllerConfig config)
    : Controller(config)
    , processor_(config.ip_, io_service_)
{
    g_elastic.log(INFO, "Creating CLBController({})", config.eid_); 
}

CLBController::~CLBController()
{
    // Empty
}

void CLBController::init()
{
    // In initialisation we test the CLB connection, set basic values
    // and set the CLB state to INIT
    g_elastic.log(DEBUG, "CLBController({}) Init...", config_.eid_); 
    testConnection();
    resetState();
    setInitValues();
    setState(CLBEvent(CLBEvents::INIT));
    g_elastic.log(DEBUG, "CLBController({}) Init DONE", config_.eid_); 
}

void CLBController::configure()
{
    // In configuration we set and check the PMT voltages and set the 
    // CLB state to CONFIGURE
    g_elastic.log(DEBUG, "CLBController({}) Configure...", config_.eid_); 
    setPMTs();
    checkPMTs();   
    setState(CLBEvent(CLBEvents::CONFIGURE));
    g_elastic.log(DEBUG, "CLBController({}) Configure DONE", config_.eid_); 
}

void CLBController::startData() 
{
    // When we start the data flow we set the CLB state to START
    g_elastic.log(DEBUG, "CLBController({}) Start Data...", config_.eid_);
    setState(CLBEvent(CLBEvents::START));
    g_elastic.log(DEBUG, "CLBController({}) Start Data DONE", config_.eid_);
}

void CLBController::stopData()
{
    // When we start the data flow we set the CLB state to STOP
    g_elastic.log(DEBUG, "CLBController({}) Stop Data...", config_.eid_); 
    setState(CLBEvent(CLBEvents::PAUSE));
    setState(CLBEvent(CLBEvents::STOP));
    setState(CLBEvent(CLBEvents::CONFIGURE));
    g_elastic.log(DEBUG, "CLBController({}) Stop Data DONE", config_.eid_);
}

void CLBController::flasherOn(float flasher_v)
{
    g_elastic.log(DEBUG, "CLBController({}) Enabling Nanobeacon...", config_.eid_);
    std::vector<int>  var_ids;
    std::vector<long> var_values;
    var_ids.push_back(ProcVar::OPT_NANO_VOLT);      var_values.push_back(60000);                       // TODO Nanobeacon Voltdage should be set somewhere 
    var_ids.push_back(ProcVar::SYS_SYS_RUN_ENA);    var_values.push_back(( 0x70000a0 | 0x8000000 ));   // | 0x8000000 Add  the Nanobeancon enabling bit to SYS_SYS_RUN_ENA 

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    for(int ii=0; ii<var_ids.size(); ++ii)
    {
        mw.writeI32(var_ids[ii]);
        mw.writeU32(var_values[ii]);
    }
 
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr); 
    sleep(1);
    g_elastic.log(DEBUG, "CLBController({}) Enabling Nanobeacon DONE", config_.eid_);  
}

void CLBController::flasherOff()
{
    g_elastic.log(DEBUG, "CLBController({}) Disabling Nanobeacon...", config_.eid_); 
    std::vector<int>  var_ids;
    std::vector<long> var_values;
    var_ids.push_back(ProcVar::SYS_SYS_RUN_ENA);    var_values.push_back(( 0x70000a0 )); // Should probably get the current status or have a value stored???

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    for(int ii=0; ii<var_ids.size(); ++ii)
    {
        mw.writeI32(var_ids[ii]);
        mw.writeU32(var_values[ii]);
    }

     
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);  
    sleep(1);
    g_elastic.log(DEBUG, "CLBController({}) Disabling Nanobeacon DONE", config_.eid_); 
}

void CLBController::testConnection()
{
    // To test the connection we ask for the date of the software revisions
    MsgWriter mw;
    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_SYS_DATEREV, mw, mr))
    {
        g_elastic.log(ERROR, "Could not reach CLB({})", config_.eid_); 
        return;
    }
    //g_elastic.log(DEBUG, "Test successful, hardware({0:8x}), software({0:8x})", mr.readU32(), mr.readU32()); 
    sleep(1);
}

void CLBController::setInitValues()
{
    std::vector<int>  var_ids;
    std::vector<long> var_values;

    unsigned char  addr4[4] = {192, 168, 11, 1};  // Server IP Address
    unsigned long ipi = ((0xFF & addr4[0]) << 24) |  ((0xFF & addr4[1]) << 16) |  ((0xFF & addr4[2]) <<  8) |  ((0xFF & addr4[3]) << 0);
    var_ids.push_back(ProcVar::NET_IPMUX_SRV_IP);       var_values.push_back(ipi);

    var_ids.push_back(ProcVar::SYS_TIME_SLICE_DUR);     var_values.push_back(config_.window_dur_);
    var_ids.push_back(ProcVar::OPT_HR_VETO_ENA_CH);     var_values.push_back(0x00000000); // Disable all Channels HR Veto  
    var_ids.push_back(ProcVar::OPT_MULHIT_ENA_CH);      var_values.push_back(0x00000000); // Disable all Channels Multi Hits

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    for(int ii=0; ii<var_ids.size(); ++ii){
        mw.writeI32(var_ids[ii]);
        mw.writeU32(var_values[ii]);
    }

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);  
    sleep(1); 
}

void CLBController::getIPMuxPorts()
{
    // First lets get all the info we want back from the CLB
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::NET_IPMUX_PORTS);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr);   
    if (int count = mr.readU16() != 1)
    {
        g_elastic.log(ERROR, "Got wrong number of return variables {}", count); 
        return;           
    }

    int varId = mr.readU32(); 
    g_elastic.log(DEBUG, "Ports: {}, {}, {}, {}", mr.readU16(), mr.readU16(), mr.readU16(), mr.readU16()); 
}

void CLBController::disableHV()
{
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::SYS_SYS_DISABLE);
    unsigned long disable = (0 | ProcVar::SYS_SYS_DISABLE_HV);
    mw.writeU32(disable);
    
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);
    sleep(1);  
}

void CLBController::resetState()
{
    // First we get the state so we know how to reset it
    getState();

    if (state_ == IDLE) {
        // Don't do anything
    } else if (state_ == STAND_BY) {
        setState(CLBEvent(CLBEvents::RESET));
    } else if (state_ == READY) {
        setState(CLBEvent(CLBEvents::QUIT));
        setState(CLBEvent(CLBEvents::RESET));
    } else if (state_ == PAUSED) {
        setState(CLBEvent(CLBEvents::STOP));
        setState(CLBEvent(CLBEvents::RESET));
    } else if (state_ == RUNNING) {
        setState(CLBEvent(CLBEvents::PAUSE));
        setState(CLBEvent(CLBEvents::STOP));
        setState(CLBEvent(CLBEvents::RESET));
    } else {
        g_elastic.log(WARNING, "CLB({}), do not know how to reset from this state", config_.eid_);
    }
}

void CLBController::setState(CLBEvent event)
{
    if (state_ != event.source_) {
        g_elastic.log(WARNING, "CLB({}) is not in the correct source state! {} {}", config_.eid_, state_, event.source_);
        return;
    }

    // First lets set the CLB state
    MsgWriter mw;
    int subsys = ClbSys::CLB_SUB_ALL;
    mw.writeI8(subsys);
    mw.writeI8(event.event_);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_EVENT, mw, mr);  

    sleep(1);
    getState();   
}

void CLBController::getState()
{
    MsgWriter mw;
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_EXT_UPDATE, mw, mr); 

    int size = mr.readU8(); 
    int currentState;
    for (int sys=0; sys<size; sys++)
    {
        int subsys  = mr.readU8();
        int state   = mr.readU8();
        int status  = mr.readU8();
        int errCode = mr.readI32();
        std::string errMsg;
        if (errCode > 0) {
            errMsg = mr.readString();
            g_elastic.log(ERROR, "CLB({}), Subsys {} in state {} has err code {}!", config_.eid_, subsys, state, errCode); 
            std::cout << errMsg << std::endl;
        } else {
            errMsg = "";
        }
        if (sys == 0) currentState = state;
        else { 
            if (currentState != state) 
            {
                g_elastic.log(ERROR, "CLB({}), Not all subsystems in the current state!", config_.eid_); 
            }
        }
    }
    state_ = (CLBState)currentState;
}

void CLBController::setPMTs()
{
    std::vector<int>  var_ids;
    std::vector<long> var_values;
    var_ids.push_back(ProcVar::OPT_CHAN_ENABLE);          
    var_ids.push_back(ProcVar::OPT_PMT_HIGHVOLT);

    unsigned long enabled = config_.chan_enabled_.to_ulong();

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    mw.writeI32(var_ids[0]);
    mw.writeU32(enabled);
    mw.writeI32(var_ids[1]);
    for(int ipmt=0; ipmt<31; ++ipmt) mw.writeU8((short)config_.chan_hv_[ipmt]);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);  
    sleep(1);    
}

void CLBController::checkPMTs()
{
    // First lets get all the info we want back from the CLB
    std::vector<int> var_ids;
    var_ids.push_back(ProcVar::OPT_CHAN_ENABLE);
    var_ids.push_back(ProcVar::OPT_PMT_ID);
    var_ids.push_back(ProcVar::OPT_PMT_HIGHVOLT);
    MsgWriter mw;
    mw.writeU16(var_ids.size());
    mw.writeI32(var_ids[0]);
    mw.writeI32(var_ids[1]);
    mw.writeI32(var_ids[2]);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr);  

    int count = mr.readU16();               
    if (count != var_ids.size())
    {
        g_elastic.log(ERROR, "Got wrong number of return variables {}", count); 
        return;           
    }

    // Check the enabled channels
    int varId    = mr.readI32();   
    std::bitset<32> enabled(mr.readU32());
    if (enabled != config_.chan_enabled_)
    {
        g_elastic.log(ERROR, "CLB({}), Enabled channels do not match!", config_.eid_);       
    }

    // Check the channel eids
    varId = mr.readI32();            
	for(int ipmt =0; ipmt<31; ++ipmt){  
        long eid = mr.readU32();
        if (eid != config_.chan_eid_[ipmt] && enabled[ipmt])
        {
            g_elastic.log(ERROR, "Non matching eid on CLB({}) for PMT {}, {} vs {}!", config_.eid_, ipmt, eid, config_.chan_eid_[ipmt]);            
        }
    }

    // Check the channel high voltage
    varId = mr.readI32();         
	for(int ipmt =0; ipmt<31; ++ipmt){
        long voltage = (long)mr.readU8();
        if (voltage != config_.chan_hv_[ipmt] && enabled[ipmt])
        {
            g_elastic.log(ERROR, "Non matching voltage on CLB({}) for PMT {}, {} vs {}!", config_.eid_, ipmt, voltage, config_.chan_hv_[ipmt]);           
        }
    }
}