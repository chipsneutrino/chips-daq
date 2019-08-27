#pragma once

using disc_t = unsigned char;

enum class RunType {
    DataNormal = 1,
    Calibration,
    TestNormal,
    TestFlasher
};

struct OpsMessage {
    disc_t Discriminator;

    /// Configure from config files
    struct Config {
        static constexpr disc_t Discriminator = 0;
        char config_file[200]; ///< Config file location
    };

    /// Start data flow
    struct StartData {
        static constexpr disc_t Discriminator = 1;
    };

    /// Stop data flow
    struct StopData {
        static constexpr disc_t Discriminator = 2;
    };

    /// Start a new data taking run
    struct StartRun {
        static constexpr disc_t Discriminator = 3;
        RunType run_type; ///< Type of run
    };

    /// Stop current data taking run
    struct StopRun {
        static constexpr disc_t Discriminator = 4;
    };

    /// Exit, possibly stopping current run
    struct Exit {
        static constexpr disc_t Discriminator = 5;
    };

    union {
        Config pConfig;
        StartData pStart;
        StopData pStop;
        StartRun pStartRun;
        StopRun pStopRun;
        Exit pExit;
    } Payload;
};

struct ControlMessage {
    const char Zero = '\0'; ///< The first bit must be '\0' otherwise NNG pub/sub discards the message.
    disc_t Discriminator;

    /// Configure from config files
    struct Config {
        static constexpr disc_t Discriminator = 0;
        char config_file[200]; ///< Config file location
    };

    /// Start data flow
    struct StartData {
        static constexpr disc_t Discriminator = 1;
    };

    /// Stop data flow
    struct StopData {
        static constexpr disc_t Discriminator = 2;
    };

    /// Start a new data taking run
    struct StartRun {
        static constexpr disc_t Discriminator = 3;
        RunType run_type; ///< Type of run
    };

    /// Stop current data taking run
    struct StopRun {
        static constexpr disc_t Discriminator = 4;
    };

    /// Exit, possibly stopping current run
    struct Exit {
        static constexpr disc_t Discriminator = 5;
    };

    union {
        Config pConfig;
        StartData pStart;
        StopData pStop;
        StartRun pStartRun;
        StopRun pStopRun;
        Exit pExit;
    } Payload;
};

struct DaqoniteStateMessage {
    const char Zero = '\0'; ///< The first bit must be '\0' otherwise NNG pub/sub discards the message.
    disc_t Discriminator;

    /// Daqonite is just sitting there doing nothing
    struct Ready {
        static constexpr disc_t Discriminator = 0;
    };

    /// Daqonite is actively processing and saving data
    struct Running {
        static constexpr disc_t Discriminator = 1;
        RunType Which;
    };

    union {
        Ready pReady;
        Running pRunning;
    } Payload;
};

struct DaqontrolStateMessage {
    const char Zero = '\0'; ///< The first bit must be '\0' otherwise NNG pub/sub discards the message.
    disc_t Discriminator;

    /// Daqontrol is initialising
    struct Initialising {
        static constexpr disc_t Discriminator = 0;
    };

    /// Daqontrol is just sitting there doing nothing
    struct Ready {
        static constexpr disc_t Discriminator = 1;
    };

    /// Daqontrol is configured
    struct Configured {
        static constexpr disc_t Discriminator = 2;
    };

    /// Daqontrol is started
    struct Started {
        static constexpr disc_t Discriminator = 3;
    };

    union {
        Initialising pInitialising;
        Ready pReady;
        Configured pConfigured;
        Started pStarted;
    } Payload;
};

struct DaqsitterStateMessage {
    const char Zero = '\0'; ///< The first bit must be '\0' otherwise NNG pub/sub discards the message.
    disc_t Discriminator;

    /// Daqsitter is just sitting there doing nothing
    struct Ready {
        static constexpr disc_t Discriminator = 0;
    };

    /// Daqsitter is monitoring packets
    struct Started {
        static constexpr disc_t Discriminator = 1;
    };

    union {
        Ready pReady;
        Started pStarted;
    } Payload;
};
