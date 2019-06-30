/**
 * DAQControl - Controller class for the CLBs and BBBs
 */

#include "daq_control.h"


DAQControl::DAQControl(std::string config_file)
    : config_(config_file.c_str())
    , processors_{}
    , n_threads_{}
    , mode_(false)
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_group_{}
{

    // Print the configuration
    config_.printShortConfig();

    setupProcessors();
}

void DAQControl::setupProcessors()
{
    // TODO: Make this configurable
  /*
    processors_.push_back(new MsgProcessor("192.168.11.11", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.12", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.14", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.15", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.16", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.17", io_service_));
    processors_.push_back(new MsgProcessor("192.168.11.18", io_service_));
  */
    /// Add test CLB only 
    processors_.push_back(new MsgProcessor("192.168.11.36", io_service_));

    // Calculate thread count
    n_threads_ = processors_.size();
}

void DAQControl::run()
{
    // Setup the thread group and call io_service.run() in each
    g_elastic.log(INFO, "DAQ Control starting I/O service on {} threads", n_threads_);
    for (int i = 0; i < n_threads_; ++i) {
        thread_group_.create_thread(boost::bind(&DAQControl::ioServiceThread, this));
    }
}

void DAQControl::ioServiceThread()
{
    io_service_->run();
}

void DAQControl::handleStartCommand(RunType which)
{
    // If we are currently running first stop the current run
    if (mode_ == true) {
        g_elastic.log(INFO, "DAQ Control stopping current run");
        handleStopCommand();
    }

    // Set the mode to data taking
    run_type_ = which;
    mode_ = true;
}

void DAQControl::handleStopCommand()
{
    // Check we are actually running
    if (mode_ == true) {
        // Set the mode to monitoring
        mode_ = false;

    } else {
        g_elastic.log(INFO, "DAQ Control already not running");
    }
}

void DAQControl::handleExitCommand()
{
    handleStopCommand();
    run_work_.reset();
    io_service_->stop();
}

void DAQControl::testMessage()
{
    // Get the Date of the hardware and software revisions from the CLBs
    MsgWriter mw;
    processors_[0]->postCommand(MsgTypes::MSG_SYS_DATEREV, mw);  
}


// Stefano's Stuff /////////////////////////

// Initialise 
void DAQControl::init(){

  setInitValues();                // Set IP address Window Width etc ..

  /// Need to check  if ready (how??).   Sleep 5 sec for now   
  sleep(5);

  // INIT CLB
  clbEvent(ClbEvents::INIT); 

}

void DAQControl::setInitValues(){
  
  /// TODOD This should be made configurable according to the Config File
  std::vector<int>  var_ids;
  std::vector<long> var_values;
 

  unsigned char  addr4[4] = {192, 168, 11, 1};  // Server IP Address
  long ipi = ((0xFF & addr4[0]) << 24) |  ((0xFF & addr4[1]) << 16) |  ((0xFF & addr4[2]) <<  8) |  ((0xFF & addr4[3]) << 0);
  //  printf(">>>>>>    IPv4 : %d   %d.%d.%d.%d\n", ipi, addr4[0],addr4[2],addr4[3],addr4[4] );

  long tw = 1000; // Time Window ms 


  var_ids.push_back(ProcVar::NET_IPMUX_SRV_IP);       var_values.push_back(ipi);
  var_ids.push_back(ProcVar::SYS_TIME_SLICE_DUR);     var_values.push_back(tw);

  var_ids.push_back(ProcVar::OPT_HR_VETO_ENA_CH);     var_values.push_back(0x00000000); // Disable all Channels HR Veto  
  var_ids.push_back(ProcVar::OPT_MULHIT_ENA_CH);      var_values.push_back(0x00000000); // Disable all Channels Multi Hits
  //var_ids.push_back(ProcVar::SYS_STMACH_PKTSIZE);

  /*  if(isLED){   // TODO Set a Flag  for LED Runs 
      g_elastic.log(INFO, "Enabling Nanobeacon");
      addNanobeacon( var_ids, var_values);  // To be completed  
      }*/	


  MsgWriter mw;
  mw.writeU16(var_ids.size());
  for(int ii=0; ii<var_ids.size(); ++ii){
    mw.writeI32(var_ids[ii]   );
    mw.writeU32(var_values[ii]);
  }
  
  g_elastic.log(DEBUG, "Setting Initial Values");  
  processors_[0]->postCommand(MsgTypes::MSG_CLB_SET_VARS, mw);  

}


void DAQControl::addNanobeacon(std::vector<int> &vid, std::vector<long> &vv){

  vid.push_back(ProcVar::OPT_NANO_VOLT);      vv.push_back(60000);                       // TODO Nanobeacon Voltdage should be set somewhere 
  vid.push_back(ProcVar::SYS_SYS_RUN_ENA);    vv.push_back(( 0x70000a0 | 0x8000000 ));   // | 0x8000000 Add  the Nanobeancon enabling bit to SYS_SYS_RUN_ENA 
                                                                                         // Should probably get the current status or have a value stored???
}



void DAQControl::disableNanobeacon(){

  std::vector<int>  var_ids;
  std::vector<long> var_values;

  var_ids.push_back(ProcVar::SYS_SYS_RUN_ENA);    var_values.push_back(( 0x70000a0 )); // Should probably get the current status or have a value stored???

  MsgWriter mw;
  mw.writeU16(var_ids.size());
  for(int ii=0; ii<var_ids.size(); ++ii){
    mw.writeI32(var_ids[ii]   );
    mw.writeU32(var_values[ii]);
  }
  
  g_elastic.log(DEBUG, "Disabling Nanobeacon");  
  processors_[0]->postCommand(MsgTypes::MSG_CLB_SET_VARS, mw);  
 
}


void DAQControl::disableHV(){

  MsgWriter mw;
  mw.writeU16(1);
  mw.writeI32(ProcVar::SYS_SYS_DISABLE);
  mw.writeU32((0 | ProcVar::SYS_SYS_DISABLE_PWR_MEAS ));
  
  g_elastic.log(DEBUG, "Disabling HV");  
  processors_[0]->postCommand(MsgTypes::MSG_CLB_SET_VARS, mw);  
  
}


void DAQControl::clbEvent(int event_id){
  
  // TODO check event id is correct. Event IDs defined in clb_events.h
 
  MsgWriter mw;
  int subsys = ClbSys::CLB_SUB_ALL;
  
  mw.writeI8(subsys);
  mw.writeI8(event_id);
  processors_[0]->postCommand(MsgTypes::MSG_CLB_EVENT, mw);    

}

void DAQControl::setPMTs(){

  /// TODO: get the PMT e-IDs and check they match with confog file. 
  /// Threshold Always the same. Just add Voltage and enabling code

  std::vector<int>  var_ids;
  std::vector<long> var_values;

  var_ids.push_back(ProcVar::OPT_CHAN_ENABLE); 	 
  var_ids.push_back(ProcVar::OPT_PMT_HIGHVOLT);
  
  short int hv[31] = {0};
  
  /// WARNING: Even if we use only 30 PMTS the array should ALWAYS contain 31 elements as the CLB expects them
  /// TODO:    Load HV and Enable from Config File
  ///          Set Just one channel for now (channel 27)  
  long enable = 1 << 27;
  hv[27] = 120;

  MsgWriter mw;
  mw.writeU16(var_ids.size());

  mw.writeI32(var_ids[0]);
  mw.writeU32(enable);

  mw.writeI32(var_ids[1]);
  for(int ipmt=0; ipmt<31; ++ipmt)  mw.writeU8( hv[ipmt]);
  processors_[0]->postCommand(MsgTypes::MSG_CLB_SET_VARS, mw);      

}


void DAQControl::checkPMTs(){} // It should check PMT info  match config file or expectations 

void DAQControl::askState(){}    /// Should ask the CLB State (Running, Paused etc.) Not sure how to do that yet 



void DAQControl::askPMTsInfo(int info_type){    // Ask for PMT Info. TODO  add method (in msg_processor ?) to actually get the Information 

  if(   info_type == ProcVar::OPT_PMT_HIGHVOLT  ||
	info_type == ProcVar::OPT_PMT_THRESHOLD ||
	info_type == ProcVar::OPT_PMT_ID        ||
	info_type == ProcVar::OPT_CHAN_ENABLE   ){

    std::vector<int> var_ids;
    var_ids.push_back(info_type);

    MsgWriter mw;
    mw.writeI32Arr(var_ids);
    processors_[0]->postCommand(MsgTypes::MSG_CLB_GET_VARS, mw);  

  } else {

    g_elastic.log(ERROR, "askPMTInfo: Wrong Variable ID!");  

  }    
  
}



void DAQControl::askVars(std::vector<int> var_ids){    // Ask Vars. TODO  add method (in msg_processor ?) to actually get the Information 

  /// TODO:  check  var ids ar correct

    MsgWriter mw;
    mw.writeI32Arr(var_ids);
    processors_[0]->postCommand(MsgTypes::MSG_CLB_GET_VARS, mw);    
}


////////////////////////////////////////////////////////


void DAQControl::join() 
{
    // Wait for all the threads to finish
    thread_group_.join_all();

    g_elastic.log(INFO, "DAQ Control finished.");
}
