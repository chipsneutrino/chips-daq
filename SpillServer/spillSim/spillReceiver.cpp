/**
 * Class NssXmlRpcServerClasses
 *
 * These classes define the XmlRpc methods that are used to send
 * information from near to far.
 * 
 * @author H. Meyer
 * @author <a href="mailto:holger.meyer@wichita.edu">holger.meyer@wichita.edu</a>
 */

#include "XmlRpc/XmlRpc.h"
#include <string>
#include <iostream>
#include <vector>
#include <list>

class Spill : public XmlRpc::XmlRpcServerMethod
{
 public:
  Spill(XmlRpc::XmlRpcServer* s);

  void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);

 private:
};

/**
 * NssSpillInfo implementation file
 * $Id: NssXmlRpcServerClasses.cpp,v 1.20 2014/03/26 16:24:54 edniner Exp $
 */

#define TRIALLIMIT 3


using namespace XmlRpc;

Spill::Spill (XmlRpcServer* s) :
  XmlRpcServerMethod("Spill", s)
{
}

// Receive Spill information and distribute through rms message
void Spill::execute(XmlRpcValue& params, XmlRpcValue& result)
{
  std::string const bad("bad");
  int nArgs = params.size();
  if (nArgs != 2) {
    /*//We expect to receive the time and type here. Something wrong...
    //ToDo: don't just quietly fail...
    result = bad;

    //std::cout << "NovaSpillServer: This is bad:" << std::endl;
    //std::cout << " got "<<nArgs<<" parameters, expected 2." << std::endl;
    for (int i = 0; i != nArgs; ++i) {
      std::cout << "Param" << i << ": " << params[i] << std::endl;
    }
    ::mf::LogError("receiverParams")<<"Expected to receiver two parameters, instead got: "<<nArgs;*/

    return;
  }

  //Should do more type checking here. The params could be anything...
  //NssSpillInfo spillInfo;
  //uint64_t ttime;
  //NssUtil::gpstimeFromString(ttime,string(params[0]));
  //int ttype = int(params[1]);
  
  std::cout << params[0] << "\t " << params[1] << std::endl;

  /*spillInfo.set(ttime,ttype);
  nssmessages::SpillMessage spillMessage;
  spillMessage.time = spillInfo.getTime();
  spillMessage.type = spillInfo.getType();
  requestSenderptr->sendMessage(spillMessage);

  //compute difference from unix system time
  timeval now;
  uint64_t unixNow;
  gettimeofday(&now,0);
  novadaq::timeutils::convertUnixTimeToNovaTime(now,unixNow);


  //make the spill receiver noisy so we know it is working
  //std::cout << "Spill message received of type: "<< spillInfo.getType() << " at time: "
    //        << spillInfo.getTime() << std::endl;
  ::mf::LogInfo("spillReceiver")<< "Received type: "<<spillInfo.getType()<<" triggerTime: "
                                <<spillInfo.getTime()<<" currentTime: "<<unixNow;*/
  //Don't wait for reply. 
  result = std::string("Ok");
}

int main(int argc, char* argv[])
{
  int port = 55812;
  XmlRpcServer s;
  Spill spill(&s);
  XmlRpc::setVerbosity(0);
  s.bindAndListen(port);
  s.work(-1.0);
  return 0;
}

