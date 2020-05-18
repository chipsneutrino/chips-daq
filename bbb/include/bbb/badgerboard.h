#pragma once

#include <cstdint>
#include <string>

#include <nngpp/nngpp.h>

class Badgerboard {
public:
    explicit Badgerboard();

    void configureHub();
    void configureRun();
    void setPowerState();
    void reprogram();
    void beginDataRun();
    void abortDataRun();
    void terminate();
    void shutdown();

private:
    void sendAndWaitForAcknowledgement(nng::buffer& request);

    std::string address_;
};