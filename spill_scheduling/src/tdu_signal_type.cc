#include "tdu_signal_type.h"

std::string getTDUSignalTypeString(TDUSignalType type)
{
    std::string ret = "Undefined";
#define NAME_CHECK(arg) \
    if (type == k##arg) \
        ret = #arg;
    NAME_CHECK(NuMI);
    NAME_CHECK(BNB);
    NAME_CHECK(NuMItclk);
    NAME_CHECK(BNBtclk);
    NAME_CHECK(AccelOneHztclk);
    NAME_CHECK(Fake);
    NAME_CHECK(TestConnection);
    NAME_CHECK(SuperCycle);
    NAME_CHECK(NuMISampleTrig);
    NAME_CHECK(NuMIReset);
    NAME_CHECK(TBSpill);
    NAME_CHECK(TBTrig);
#undef NAME_CHECK
    return ret;
}