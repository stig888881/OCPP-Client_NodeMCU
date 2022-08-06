// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#ifndef OCPPMODEL_H
#define OCPPMODEL_H

#include <ArduinoOcpp/Core/OcppTime.h>

#include <memory>

namespace ArduinoOcpp {

class SmartChargingService;
class ChargePointStatusService;
class ConnectorStatus;
class MeteringService;
class FirmwareService;
class DiagnosticsService;
class HeartbeatService;

class OcppModel {
private:
    std::unique_ptr<SmartChargingService> smartChargingService;
    std::unique_ptr<ChargePointStatusService> chargePointStatusService;
    std::unique_ptr<MeteringService> meteringService;
    std::unique_ptr<FirmwareService> firmwareService;
    std::unique_ptr<DiagnosticsService> diagnosticsService;
    std::unique_ptr<HeartbeatService> heartbeatService;
    OcppTime ocppTime;

public:
    OcppModel(const OcppClock& system_clock);
    OcppModel() = delete;
    OcppModel(const OcppModel& rhs) = delete;
    ~OcppModel();

    void loop();

    void setSmartChargingService(std::unique_ptr<SmartChargingService> scs);
    SmartChargingService* getSmartChargingService() const;

    void setChargePointStatusService(std::unique_ptr<ChargePointStatusService> cpss);
    ChargePointStatusService *getChargePointStatusService() const;
    ConnectorStatus *getConnectorStatus(int connectorId) const;

    void setMeteringSerivce(std::unique_ptr<MeteringService> meteringService);
    MeteringService* getMeteringService() const;

    void setFirmwareService(std::unique_ptr<FirmwareService> firmwareService);
    FirmwareService *getFirmwareService() const;

    void setDiagnosticsService(std::unique_ptr<DiagnosticsService> diagnosticsService);
    DiagnosticsService *getDiagnosticsService() const;

    void setHeartbeatService(std::unique_ptr<HeartbeatService> heartbeatService);

    OcppTime &getOcppTime();
};

} //end namespace ArduinoOcpp

#endif
