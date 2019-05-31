#pragma once

using disc_t = unsigned char;

enum class RunType
{
    DataNormal = 1,
    Calibration,
    TestNormal,
    TestDAQ
};

struct OpsMessage
{
    static const char * const URL;

    disc_t Discriminator;
    struct StartRun
    {
        static constexpr disc_t Discriminator = 0;
    };
    struct StopRun
    {
        static constexpr disc_t Discriminator = 1;
    };
};

struct ControlMessage
{
    static const char * const URL;
    disc_t Discriminator;

    /// Start a new data run
    struct StartRun
    {
        static constexpr disc_t Discriminator = 0;
        RunType Which;
    };

    /// Stop current run
    struct StopRun
    {
        static constexpr disc_t Discriminator = 1;
    };

    /// Exit, possibly stopping current run
    struct Exit
    {
        static constexpr disc_t Discriminator = 2;
    };

    union {
        StartRun pStartRun;
        StopRun pStopRun;
        Exit pExit;
    } Payload;
};

struct DaqoniteStateMessage
{
    static const char * const URL;
    //disc_t Discriminator;

    bool RunMode;

    /// Daqonite is just sitting there doing nothing
    /*struct Idle {
        static constexpr disc_t Discriminator = 0;
    };

    /// Daqonite is actively processing and saving data
    struct RunInProgress {
        static constexpr disc_t Discriminator = 1;
        RunType Which;
    };

    union {
        Idle pIdle;
        RunInProgress pRunInProgress;
    } Payload;*/
};
