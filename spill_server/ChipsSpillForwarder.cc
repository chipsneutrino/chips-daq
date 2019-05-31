// System Libs
#include <iostream>

// External Libs
#include <boost/program_options.hpp>

// Custom Libs
#include "NovaSpillServer/NssXmlRpcServerClasses.h"
#include "NovaSpillServer/NssSpillInfo.h"

using namespace XmlRpc;
namespace po = boost::program_options;

// The server
XmlRpcServer s;

// The spills
//Spill spill(&s, requestSenderPtr(p));

int main(int argc, char* argv[])
{

  int receivePort = 2718;
  int targetPort = 3141;
  std::string targetAddr = "127.0.0.1";
 
  // Declare the supported command line options.
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "usage descriptions")
    ("port,p", po::value<int>(&receivePort)->default_value(2718), "The port number to receive spills.")
    ("target,t", po::value<int>(&targetPort)->default_value(3141),"The target port to send spills.")
    ("address,a", po::value<std::string>(&targetAddr)->default_value("127.0.0.1"),"The target address to send the received spill to.");
  
  po::positional_options_description p;

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(),vm);
  po::notify(vm);

  if (vm.count("help") || (receivePort == -1))
  {
    std::cout << desc << std::endl;
    return 1;
  }

  if (vm.count("help") || (targetPort == -1))
  {
  std::cout << desc << std::endl;
  return 1;
  }

  if ( (receivePort < 1024) || (receivePort >= 65535) )
  {
    std::cout << "Choose a port between 1024 and 65,535 for receiving." << std::endl;
    return 1;
  }

  if ( (targetPort < 1024 ) || ( targetPort >= 65525) )
  {
    std::cout << "Choose a target port between 1024 and 65,535 for sending." << std::endl;
    return 1;
  }

  std::cout << "Listening on port: " << receivePort << "." << std::endl;
  std::cout << "Target port: " << targetPort << "." << std::endl;
  std::cout << "Target ip address: " << targetAddr << "." << std::endl;


  s.bindAndListen(receivePort);
  XmlRpc::setVerbosity(5);
  //spillInfo.getTime();

  s.work(-1.0);

  return 0;
}
