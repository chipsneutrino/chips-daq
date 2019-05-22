// NssFakeSpillSender : A simple xmlrpc client to send fake spill data.
#include "NovaSpillServer/NssSpillInfo.h"
#include "NovaSpillServer/NssUtil.h"
#include "XmlRpc/XmlRpc.h"
#include "NovaTimingUtilities/TimingUtilities.h"
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>  /* atoi */

using namespace XmlRpc;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  std::string host;
  int port;
  int verbose;
  int type;
  int count;
  int delay;
  // Parse command line options using Boost

  // Declare the supported options
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "usage descriptions")
    ("host,o", po::value<std::string>(&host)->default_value("localhost"),
     "The xmlrpc hostname")
    ("port,p", po::value<int>(&port)->default_value(55812), "the xmlrpc server's port number")
    ("verbose,v", po::value<int>(&verbose)->default_value(0), "verbosity level")
    ("type,t", po::value<int>(&type)->default_value(1), "trigger type: 1 (NuMI), 2 (BnB), 6 (PPS), 44 (TBSpill), or 45 (TBTrig)")
    ("count,c", po::value<int>(&count)->default_value(1), "how many triggers to issue")
    ("delay,d", po::value<int>(&delay)->default_value(1), "how long to pause between multiple triggers in microseconds")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help")) {
    std::cout << desc << "\n";
    exit(1);
  }


  XmlRpc::setVerbosity(verbose);
  XmlRpcClient c(host.c_str(), port);
  XmlRpcValue result;

  uint64_t gpsTime;
  std::string st;
  XmlRpcValue numbers;
  if (type == 1)
    numbers[1] = nova::NovaSpillServer::kNuMI;
  else if (type == 2)
    numbers[1] = nova::NovaSpillServer::kBNB;
  else if (type == 6)
    numbers[1] = nova::NovaSpillServer::kAccelOneHztclk;
  else if (type == 44)
    numbers[1] = nova::NovaSpillServer::kTBSpill;
  else if (type == 45)
    numbers[1] = nova::NovaSpillServer::kTBTrig;
  else 
    numbers[1] = nova::NovaSpillServer::kFake;

  std::cout << "Sending spills..." << std::endl;

  for (int i = 0; i<count; ++i) {

    timeval now;
    gettimeofday(&now,0);
    novadaq::timeutils::convertUnixTimeToNovaTime(now,gpsTime);

    {
      timeval testTv;
      novadaq::timeutils::convertNovaTimeToUnixTime(gpsTime,testTv);
      time_t testTime = testTv.tv_sec;
      char ctimestr[27];ctime_r(&testTime,ctimestr);
      std::cout << numbers[1] << " " << "at time " << ctimestr << std::endl;
    }

    nova::NovaSpillServer::NssUtil::gpstimeToString(gpsTime, st);
    numbers[0] = st;
    if (c.execute("Spill", numbers, result)) {
      std::cout << "Spill method result: " << result << "\n";
    }
    else {
      std::cout << "Error calling 'Spill'\n";
    }
    if ((i+1)< count) usleep(delay);
  }
 
  return 0;
}
