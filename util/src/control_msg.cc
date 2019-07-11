#include "control_msg.h"

const char *const OpsMessage::URL = "ipc:///tmp/chips_ops.ipc";
const char *const ControlMessage::URL = "ipc:///tmp/chips_control.ipc";
const char *const DaqoniteStateMessage::URL = "ipc:///tmp/chips_daqonite.ipc";
const char *const DaqontrolStateMessage::URL = "ipc:///tmp/chips_daqontrol.ipc";
const char *const DaqsitterStateMessage::URL = "ipc:///tmp/chips_daqsitter.ipc";