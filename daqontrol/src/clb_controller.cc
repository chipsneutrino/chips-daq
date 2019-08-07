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

void CLBController::init()
{
    g_elastic.log(DEBUG, "CLBController({}) Init...", config_.eid_); 
    working_ = true; // Set this controller to be working
    
    if(!testConnection()) { // Test the connection to the CLB
        working_ = false;
        return;
    } else if(!resetState()) { // Reset the state of the CLB to IDLE
        working_ = false;
        return;        
    } else if(!setInitValues()) { // Set the initialisation variables
        working_ = false;
        return; 
    } else if(!setState(CLBEvent(CLBEvents::INIT))) { // Set the CLB state to STAND_BY
        working_ = false;
        return; 
    }

    state_ = Control::Ready; // Set the controller state to Ready
    g_elastic.log(DEBUG, "CLBController({}) Init DONE", config_.eid_); 
    working_ = false; 
}

void CLBController::configure()
{
    g_elastic.log(DEBUG, "CLBController({}) Configure...", config_.eid_); 
    working_ = true; // Set this controller to be working

    if (clb_state_ == READY) { // We have already configured this controller
        state_ = Control::Ready; // Set the controller state to Ready
        if(!setState(CLBEvent(CLBEvents::QUIT))) { // Set the CLB state to STAND_BY
            working_ = false;
            return;    
        } 
    }

    if(!setPMTs()) { // Set and check the PMT voltages
        working_ = false;
        return;    
    }                                  
    else if(!setFlasher()) { // Set the flasher if required
        working_ = false;
        return;    
    }                               
    else if(!setState(CLBEvent(CLBEvents::CONFIGURE))) { // Set the CLB state to READY
        working_ = false;
        return;    
    }   

    state_ = Control::Configured; // Set the controller state to Configured
    g_elastic.log(DEBUG, "CLBController({}) Configure DONE", config_.eid_); 
    working_ = false;
}

void CLBController::startData() 
{
    g_elastic.log(DEBUG, "CLBController({}) Start Data...", config_.eid_);
    working_ = true; // Set this controller to be working

    if(!setState(CLBEvent(CLBEvents::START))) { // Set the CLB state to RUNNING
        working_ = false;
        return;          
    }       

    state_ = Control::Started; // Set the controller state to Started
    g_elastic.log(DEBUG, "CLBController({}) Start Data DONE", config_.eid_);
    working_ = false;
}

void CLBController::stopData()
{
    g_elastic.log(DEBUG, "CLBController({}) Stop Data...", config_.eid_); 
    working_ = true; // Set this controller to be working

    if(!setState(CLBEvent(CLBEvents::PAUSE))) { // Set the CLB state to PAUSED
        working_ = false;
        return;   
    }       
    else if(!setState(CLBEvent(CLBEvents::STOP))) { // Set the CLB state to STAND_BY
        working_ = false;
        return;   
    }        
    else if(!setState(CLBEvent(CLBEvents::CONFIGURE))) { // Set the CLB state to READY
        working_ = false;
        return;   
    }   

    state_ = Control::Configured; // Set the controller state to Configured
    g_elastic.log(DEBUG, "CLBController({}) Stop Data DONE", config_.eid_);
    working_ = false;
}

bool CLBController::testConnection()
{
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::SYS_DOM_ID);

    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'testConnection'", config_.eid_); 
        return false;
    }  
         
    if (int count = mr.readU16() != 1)
    {
        g_elastic.log(ERROR, "Got wrong number of return variables {}", count); 
        return false;           
    }

    int varId = mr.readI32();   
    long eid = mr.readU32();

    if (eid != config_.eid_)
    {
        g_elastic.log(ERROR, "CLB({}), Does not match eid {}", config_.eid_, eid); 
        return false;        
    }

    return true;
}

bool CLBController::setInitValues()
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
    mw.writeU16(var_ids.size() + 2);
    for(int ii=0; ii<var_ids.size(); ++ii){
        mw.writeI32(var_ids[ii]);
        mw.writeU32(var_values[ii]);
    }

    mw.writeI32(ProcVar::SYS_SYS_DISABLE);  mw.writeU8(0); // Make sure the AHRS is not disabled
    mw.writeI32(ProcVar::SYS_SYS_RUN_ENA);  mw.writeU8(0 | ProcVar::SYS_SYS_RUN_ENA_TDC | ProcVar::SYS_SYS_RUN_ENA_MON); // Disable the acoustic data

    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'setInitValues'", config_.eid_); 
        return false;
    }

    return true;
}

bool CLBController::resetState()
{
    if(!getState()) return false; // Update the locally stored CLB state

    if (clb_state_ == IDLE) {
        return true;
    } else if (clb_state_ == STAND_BY) {
        if (!setState(CLBEvent(CLBEvents::RESET)))  return false;
    } else if (clb_state_ == READY) {
        if (!setState(CLBEvent(CLBEvents::QUIT)))   return false;
        if (!setState(CLBEvent(CLBEvents::RESET)))  return false;
    } else if (clb_state_ == PAUSED) {
        if (!setState(CLBEvent(CLBEvents::STOP)))   return false;
        if (!setState(CLBEvent(CLBEvents::RESET)))  return false;
    } else if (clb_state_ == RUNNING) {
        if (!setState(CLBEvent(CLBEvents::PAUSE)))  return false;
        if (!setState(CLBEvent(CLBEvents::STOP)))   return false;
        if (!setState(CLBEvent(CLBEvents::RESET)))  return false;
    } else {
        g_elastic.log(WARNING, "CLB({}), do not know how to reset from this state", config_.eid_);
        return false;
    }

    return true;
}

bool CLBController::setState(CLBEvent event)
{
    if (clb_state_ != event.source_) {
        g_elastic.log(WARNING, "CLB({}) is not in the correct source state! {} {}", config_.eid_, clb_state_, event.source_);
        return false;
    }

    MsgWriter mw;
    mw.writeI8(ClbSys::CLB_SUB_ALL);    mw.writeI8(event.event_);

    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_EVENT, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'setState'", config_.eid_); 
        return false;
    }

    if(!getState()) return false; // Update the locally stored CLB state

    return true;
}

bool CLBController::getState()
{
    MsgWriter mw; MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_EXT_UPDATE, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'getState'", config_.eid_); 
        return false;
    }

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
            g_elastic.log(ERROR, "CLB({}), Subsys {} in state {} has err code {} with message {}!", config_.eid_, subsys, state, errCode, errMsg); 
            return false;
        } else {
            errMsg = "";
        }
        if (sys == 0) currentState = state;
        else { 
            if (currentState != state) 
            {
                g_elastic.log(ERROR, "CLB({}), Not all subsystems in the current state!", config_.eid_); 
                return false;
            }
        }
    }
    clb_state_ = (CLBState)currentState;
    return true;
}

bool CLBController::setPMTs()
{
    unsigned long enabled = config_.chan_enabled_.to_ulong();

    MsgWriter mw;
    mw.writeU16(2);
    mw.writeI32(ProcVar::OPT_CHAN_ENABLE);  mw.writeU32(enabled);
    mw.writeI32(ProcVar::OPT_PMT_HIGHVOLT);
    for(int ipmt=0; ipmt<31; ++ipmt) mw.writeU8((short)config_.chan_hv_[ipmt]);

    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'setPMTs'", config_.eid_); 
        return false;
    }

    if(!checkPMTs()) return false; // Check the PMTs have been set correctly  

    return true;  
}

bool CLBController::checkPMTs()
{
    // First lets get all the info we want back from the CLB
    MsgWriter mw;
    mw.writeU16(3);
    mw.writeI32(ProcVar::OPT_CHAN_ENABLE);
    mw.writeI32(ProcVar::OPT_PMT_ID);
    mw.writeI32(ProcVar::OPT_PMT_HIGHVOLT);

    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'checkPMTs'", config_.eid_); 
        return false;
    } 
             
    if (int count = mr.readU16() != 3)
    {
        g_elastic.log(ERROR, "Got wrong number of return variables {}", count); 
        return false;           
    }

    // Check the enabled channels
    int varId = mr.readI32();   
    std::bitset<32> enabled(mr.readU32());
    if (enabled != config_.chan_enabled_)
    {
        g_elastic.log(ERROR, "CLB({}), Enabled channels do not match!", config_.eid_);  
        return false;     
    }

    // Check the channel eids
    varId = mr.readI32();            
	for(int ipmt =0; ipmt<31; ++ipmt){  
        long eid = mr.readU32();
        if (eid != config_.chan_eid_[ipmt] && enabled[ipmt])
        {
            g_elastic.log(ERROR, "Non matching eid on CLB({}) for PMT {}, {} vs {}!", config_.eid_, ipmt, eid, config_.chan_eid_[ipmt]);  
            return false;          
        }
    }

    // Check the channel high voltage
    varId = mr.readI32();         
	for(int ipmt =0; ipmt<31; ++ipmt){
        long voltage = (long)mr.readU8();
        if (voltage != config_.chan_hv_[ipmt] && enabled[ipmt])
        {
            g_elastic.log(ERROR, "Non matching voltage on CLB({}) for PMT {}, {} vs {}!", config_.eid_, ipmt, voltage, config_.chan_hv_[ipmt]); 
            return false;          
        }
    }

    return true;
}

char CLBController::getSysEnabledMask()
{
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::SYS_SYS_RUN_ENA);

    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'getSysEnabledMask'", config_.eid_); 
        return -1;
    } 
         
    if (int count = mr.readU16() != 1)
    {
        g_elastic.log(ERROR, "Got wrong number of return variables {}", count); 
        return -1;           
    }

    int varId = mr.readI32();   
    char enabled = mr.readU8();
    //std::bitset<8> set(enabled);
    //std::cout << "Enabled:" << set.to_string() << std::endl;  
    return enabled;
}

char CLBController::getSysDisabledMask()
{
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::SYS_SYS_DISABLE);

    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'getSysDisabledMask'", config_.eid_); 
        return -1;
    }  
         
    if (int count = mr.readU16() != 1)
    {
        g_elastic.log(ERROR, "Got wrong number of return variables {}", count); 
        return -1;           
    }

    int varId = mr.readI32();   
    char disabled = mr.readU8();
    //std::bitset<8> set(disabled);
    //std::cout << "Disabled:" << set.to_string() << std::endl;  
    return disabled;
}

bool CLBController::setFlasher()
{
    if (config_.nano_enabled_)
    {
        MsgWriter mw;
        mw.writeU16(2);  
        mw.writeI32(ProcVar::OPT_NANO_VOLT);    mw.writeU16(config_.nano_voltage_);
        mw.writeI32(ProcVar::SYS_SYS_RUN_ENA);  
        mw.writeU8(0 | ProcVar::SYS_SYS_RUN_ENA_TDC | ProcVar::SYS_SYS_RUN_ENA_MON | ProcVar::SYS_SYS_RUN_ENA_NANO);    

        MsgReader mr;
        if(!processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr))
        {
            g_elastic.log(ERROR, "CLB({}), Could not process 'setFlasher'", config_.eid_); 
            return false;
        }    

        if(!checkFlasherVoltage()) return false;

        g_elastic.log(ERROR, "CLB({}), Flasher on at ({})", config_.eid_, config_.nano_voltage_);
    } 
    else 
    {
        MsgWriter mw;
        mw.writeU16(1);
        mw.writeI32(ProcVar::SYS_SYS_RUN_ENA);  mw.writeU8(0 | ProcVar::SYS_SYS_RUN_ENA_TDC | ProcVar::SYS_SYS_RUN_ENA_MON);
        
        MsgReader mr;
        if(!processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr))
        {
            g_elastic.log(ERROR, "CLB({}), Could not process 'setFlasher'", config_.eid_); 
            return false;
        }           
    }

    return true;
}

bool CLBController::checkFlasherVoltage()
{
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::OPT_NANO_VOLT);

    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'checkFlasherVoltage'", config_.eid_); 
        return false;
    } 
         
    if (int count = mr.readU16() != 1)
    {
        g_elastic.log(ERROR, "Got wrong number of return variables {}", count); 
        return false;           
    }

    int varId = mr.readI32();   
    float voltage = (float)mr.readU16();
    if ((config_.nano_voltage_) != voltage) 
    {
        g_elastic.log(ERROR, "The nanobeacon voltage has not been set to {} correctly it is {}", config_.nano_voltage_, voltage); 
        return false;             
    }

    return true;
}

bool CLBController::enableHV()
{
    g_elastic.log(DEBUG, "CLBController({}) Enabling High Voltage..", config_.eid_);

    char disabled;
    if(disabled = getSysDisabledMask() == -1) return false; // get the current SYS_SYS_DISABLE mask
    disabled &= ~(ProcVar::SYS_SYS_DISABLE_HV);

    MsgWriter mw;
    mw.writeU16(1);  
    mw.writeI32(ProcVar::SYS_SYS_DISABLE);  mw.writeU8(disabled);
 
    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'enableHV'", config_.eid_); 
        return false;
    }   

    g_elastic.log(DEBUG, "CLBController({}) Enabling High Voltage DONE", config_.eid_);
    return true;
}

bool CLBController::disableHV()
{
    g_elastic.log(DEBUG, "CLBController({}) Disabling High Voltage..", config_.eid_);

    char disabled;
    if(disabled = getSysDisabledMask() == -1) return false; // get the current SYS_SYS_DISABLE mask
    disabled |= ProcVar::SYS_SYS_DISABLE_HV;

    MsgWriter mw;
    mw.writeU16(1);  
    mw.writeI32(ProcVar::SYS_SYS_DISABLE);  mw.writeU8(disabled);
 
    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'disableHV'", config_.eid_); 
        return false;
    }   

    g_elastic.log(DEBUG, "CLBController({}) Disabling High Voltage DONE", config_.eid_);

    return true;
}

bool CLBController::getIPMuxPorts()
{
    // First lets get all the info we want back from the CLB
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::NET_IPMUX_PORTS);

    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr))
    {
        g_elastic.log(ERROR, "CLB({}), Could not process 'getIPMuxPorts'", config_.eid_); 
        return false;
    }       

    if (int count = mr.readU16() != 1)
    {
        g_elastic.log(ERROR, "Got wrong number of return variables {}", count); 
        return false;           
    }

    int varId = mr.readU32(); 
    g_elastic.log(DEBUG, "Ports: {}, {}, {}, {}", mr.readU16(), mr.readU16(), mr.readU16(), mr.readU16()); 

    return true;
}