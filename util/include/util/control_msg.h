#pragma once

using disc_t = unsigned char;

enum class RunType {
    DataNormal = 1,
    Calibration,
    TestNormal,
    TestFlasher
};

struct OpsMessage {
    static const char* const URL;

    disc_t Discriminator;

    /// Configure from config files
    struct Config {
        static constexpr disc_t Discriminator = 0;
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
        RunType Which;      ///< Type of run
    };

    /// Stop current data taking run
    struct StopRun {
        static constexpr disc_t Discriminator = 4;
    };

    union {
        Config pConfig;
        StartData pStart;
        StopData pStop;
        StartRun pStartRun;
        StopRun pStopRun;
    } Payload;
};

struct ControlMessage {
    static const char* const URL;

    const char Zero = '\0'; ///< The first bit must be '\0' otherwise NNG pub/sub discards the message.
    disc_t Discriminator;

    /// Configure from config files
    struct Config {
        static constexpr disc_t Discriminator = 0;
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
        RunType Which;      ///< Type of run
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
    } Payload;
};

struct DaqoniteStateMessage {
    static const char* const URL;

    const char Zero = '\0'; ///< The first bit must be '\0' otherwise NNG pub/sub discards the message.
    disc_t Discriminator;

    /// Daqonite is just sitting there doing nothing
    struct Idle {
        static constexpr disc_t Discriminator = 0;
    };

    /// Daqonite is actively processing and saving data
    struct Mining {
        static constexpr disc_t Discriminator = 1;
        RunType Which;
    };

    union {
        Idle pIdle;
        Mining pMining;
    } Payload;
};

struct DaqontrolStateMessage {
    static const char* const URL;

    const char Zero = '\0'; ///< The first bit must be '\0' otherwise NNG pub/sub discards the message.
    disc_t Discriminator;

    /// Daqontrol is initialising
    struct Initialising {
        static constexpr disc_t Discriminator = 0;
    };

    /// Daqontrol is just sitting there doing nothing
    struct Idle {
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
        Idle pIdle;
        Configured pConfigured;
        Started pStarted;
    } Payload;
};

struct DaqsitterStateMessage {
    static const char* const URL;

    const char Zero = '\0'; ///< The first bit must be '\0' otherwise NNG pub/sub discards the message.
    disc_t Discriminator;

    /// Daqsitter is just sitting there doing nothing
    struct Idle {
        static constexpr disc_t Discriminator = 0;
    };

    /// Daqsitter is monitoring packets
    struct Monitoring {
        static constexpr disc_t Discriminator = 1;
    };

    union {
        Idle pIdle;
        Monitoring pMonitoring;
    } Payload;
};
