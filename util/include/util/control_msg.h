#pragma once

using disc_t = unsigned char;

enum class RunType {
    DataNormal = 1,
    Calibration,
    TestNormal,
    TestDAQ
};

struct OpsMessage {
    static const char* const URL;

    disc_t Discriminator;
    struct StartRun {
        static constexpr disc_t Discriminator = 0;
        RunType Which;
    };
    struct StopRun {
        static constexpr disc_t Discriminator = 1;
    };

    union {
        StartRun pStartRun;
        StopRun pStopRun;
    } Payload;
};

struct ControlMessage {
    static const char* const URL;

    const char Zero = '\0'; ///< The first bit must be '\0' otherwise NNG pub/sub discards the message.
    disc_t Discriminator;

    /// Start a new data run
    struct StartRun {
        static constexpr disc_t Discriminator = 0;
        RunType Which;
    };

    /// Stop current run
    struct StopRun {
        static constexpr disc_t Discriminator = 1;
    };

    /// Exit, possibly stopping current run
    struct Exit {
        static constexpr disc_t Discriminator = 2;
    };

    union {
        StartRun pStartRun;
        StopRun pStopRun;
        Exit pExit;
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
