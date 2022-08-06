// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#ifndef UNLOCKCONNECTOR_H
#define UNLOCKCONNECTOR_H

#include <ArduinoOcpp/Core/OcppMessage.h>

namespace ArduinoOcpp {
namespace Ocpp16 {

class UnlockConnector : public OcppMessage {
private:
    bool err = false;
    bool cbDefined = false;
    bool cbUnlockSuccessful = false;
public:
    UnlockConnector();

    const char* getOcppOperationType();

    void processReq(JsonObject payload);

    std::unique_ptr<DynamicJsonDocument> createConf();
};

} //end namespace Ocpp16
} //end namespace ArduinoOcpp
#endif
