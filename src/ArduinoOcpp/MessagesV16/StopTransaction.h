// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#ifndef STOPTRANSACTION_H
#define STOPTRANSACTION_H

#include <ArduinoOcpp/Core/OcppMessage.h>
#include <ArduinoOcpp/Core/OcppTime.h>

namespace ArduinoOcpp {
namespace Ocpp16 {

class StopTransaction : public OcppMessage {
private:
    int connectorId = 1;
    int meterStop = -1;
    OcppTimestamp otimestamp;
public:

    StopTransaction(int connectorId);

    const char* getOcppOperationType();

    void initiate();

    std::unique_ptr<DynamicJsonDocument> createReq();

    void processConf(JsonObject payload);

    void processReq(JsonObject payload);

    std::unique_ptr<DynamicJsonDocument> createConf();
};

} //end namespace Ocpp16
} //end namespace ArduinoOcpp
#endif
