#pragma once

#include <memory>

#include <XmlRpc.h>
#include <util/logging.h>

#include <spill_scheduling/trigger_predictor.h>

class XMLRPCSpillMethod : public XmlRpc::XmlRpcServerMethod, protected Logging {
    std::shared_ptr<TriggerPredictor> predictor_; ///< Spill interval predictor.

public:
    XMLRPCSpillMethod(XmlRpc::XmlRpcServer* server, std::shared_ptr<TriggerPredictor> predictor);

    /// Receive spill XML-RPC message.
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
};
