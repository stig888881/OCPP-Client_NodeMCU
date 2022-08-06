// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#include <ArduinoOcpp/Tasks/Heartbeat/HeartbeatService.h>
#include <ArduinoOcpp/Core/OcppEngine.h>
#include <ArduinoOcpp/SimpleOcppOperationFactory.h>
#include <ArduinoOcpp/Core/Configuration.h>
#include <ArduinoOcpp/MessagesV16/Heartbeat.h>
#include <ArduinoOcpp/Platform.h>

using namespace ArduinoOcpp;

HeartbeatService::HeartbeatService(OcppEngine& context) : context(context) {
    heartbeatInterval = declareConfiguration("HeartbeatInterval", 86400);
    lastHeartbeat = ao_tick_ms();
}

void HeartbeatService::loop() {
    ulong hbInterval = *heartbeatInterval;
    hbInterval *= 1000UL; //conversion s -> ms
    ulong now = ao_tick_ms();

    if (now - lastHeartbeat >= hbInterval) {
        lastHeartbeat = now;

        auto heartbeat = makeOcppOperation("Heartbeat");
        context.initiateOperation(std::move(heartbeat));
    }
}
