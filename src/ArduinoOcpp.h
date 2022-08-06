// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#ifndef ARDUINOOCPP_H
#define ARDUINOOCPP_H

#include <ArduinoJson.h>
#include <memory>
#include <functional>

#include <ArduinoOcpp/Core/ConfigurationOptions.h>
#include <ArduinoOcpp/Core/OcppTime.h>
#include <ArduinoOcpp/Core/OcppOperationCallbacks.h>
#include <ArduinoOcpp/Core/OcppOperationTimeout.h>
#include <ArduinoOcpp/Core/OcppSocket.h>

using ArduinoOcpp::OnReceiveConfListener;
using ArduinoOcpp::OnReceiveReqListener;
using ArduinoOcpp::OnSendConfListener;
using ArduinoOcpp::OnAbortListener;
using ArduinoOcpp::OnTimeoutListener;
using ArduinoOcpp::OnReceiveErrorListener;

using ArduinoOcpp::Timeout;

#ifndef AO_CUSTOM_WS
//uses links2004/WebSockets library
void OCPP_initialize(const char *CS_hostname, uint16_t CS_port, const char *CS_url, float V_eff = 230.f /*German grid*/, ArduinoOcpp::FilesystemOpt fsOpt = ArduinoOcpp::FilesystemOpt::Use_Mount_FormatOnFail, ArduinoOcpp::OcppClock system_time = ArduinoOcpp::Clocks::DEFAULT_CLOCK);
#endif

//Lets you use your own WebSocket implementation
void OCPP_initialize(ArduinoOcpp::OcppSocket& ocppSocket, float V_eff = 230.f /*German grid*/, ArduinoOcpp::FilesystemOpt fsOpt = ArduinoOcpp::FilesystemOpt::Use_Mount_FormatOnFail, ArduinoOcpp::OcppClock system_time = ArduinoOcpp::Clocks::DEFAULT_CLOCK);

//experimental; More testing required (help needed: it would be awesome if you can you publish your evaluation results on the GitHub page)
void OCPP_deinitialize();

void OCPP_loop();

/*
 * Provide hardware-related information to the library
 * 
 * The library needs a way to obtain information from your charger that changes over time. The
 * library calls following callbacks regularily (if they were set) to refresh its internal
 * status.
 * 
 * Set the callbacks once in your setup() function.
 */

void setPowerActiveImportSampler(std::function<float()> power);

void setEnergyActiveImportSampler(std::function<float()> energy);

void setEvRequestsEnergySampler(std::function<bool()> evRequestsEnergy);

void setConnectorEnergizedSampler(std::function<bool()> connectorEnergized);

void setConnectorPluggedSampler(std::function<bool()> connectorPlugged);

//void setConnectorFaultedSampler(std::function<bool()> connectorFailed);

void addConnectorErrorCodeSampler(std::function<const char *()> connectorErrorCode);

/*
 * React on calls by the library's internal functions
 * 
 * The library needs to set parameters on your charger on a regular basis or perform
 * functions triggered by the central system. The library uses the following callbacks
 * (if they were set) to perform updates or functions on your charger.
 * 
 * Set the callbacks once in your setup() function.
 */

void setOnChargingRateLimitChange(std::function<void(float)> chargingRateChanged);

void setOnUnlockConnector(std::function<bool()> unlockConnector); //true: success, false: failure

/*
 * React on CS-initiated operations
 * 
 * You can define custom behaviour in your integration which is executed every time the library
 * receives a CS-initiated operation. The following functions set a callback function for the
 * respective event. The library executes the callbacks always after its internal routines.
 * 
 * Set the callbacks once in your setup() function.
 */

void setOnSetChargingProfileRequest(OnReceiveReqListener onReceiveReq); //optional

void setOnRemoteStartTransactionSendConf(OnSendConfListener onSendConf); //important, energize the power plug here and capture the idTag

void setOnRemoteStopTransactionSendConf(OnSendConfListener onSendConf); //important, de-energize the power plug here
void setOnRemoteStopTransactionReceiveReq(OnReceiveReqListener onReceiveReq); //optional, to de-energize the power plug immediately

void setOnResetSendConf(OnSendConfListener onSendConf); //important, reset your device here (i.e. call ESP.reset();)
void setOnResetReceiveReq(OnReceiveReqListener onReceiveReq); //alternative: start reset timer here

/*
 * Perform CP-initiated operations
 * 
 * Use following functions to send OCPP messages. Each function will adapt the library's
 * internal state appropriatley (e.g. after a successful StartTransaction request the library
 * will store the transactionId and send a StatusNotification).
 * 
 * On receipt of the .conf() response the library calls the callback function
 * "OnReceiveConfListener onConf" and passes the OCPP payload to it. The following functions
 * are non-blocking. Your application code will immediately resume with the subsequent code
 * in any case.
 * 
 * For your first EVSE integration, the `onReceiveConfListener` is probably sufficient. For
 * advanced EVSE projects, the other listeners likely become relevant:
 * - `onAbortListener`: will be called whenever the engine stops trying to finish an operation
 *           normally which was initiated by this device.
 * - `onTimeoutListener`: will be executed when the operation is not answered until the timeout
 *           expires. Note that timeouts also trigger the `onAbortListener`.
 * - `onReceiveErrorListener`: will be called when the Central System returns a CallError.
 *           Again, each error also triggers the `onAbortListener`.
 */

void authorize(const char *idTag, OnReceiveConfListener onConf = nullptr, OnAbortListener onAbort = nullptr, OnTimeoutListener onTimeout = nullptr, OnReceiveErrorListener onError = nullptr, std::unique_ptr<Timeout> timeout = nullptr);

void bootNotification(const char *chargePointModel, const char *chargePointVendor, OnReceiveConfListener onConf = nullptr, OnAbortListener onAbort = nullptr, OnTimeoutListener onTimeout = nullptr, OnReceiveErrorListener onError = nullptr, std::unique_ptr<Timeout> timeout = nullptr);

//The OCPP operation will include the given payload without modifying it. The library will delete the payload object after successful transmission.
void bootNotification(DynamicJsonDocument *payload, OnReceiveConfListener onConf = nullptr, OnAbortListener onAbort = nullptr, OnTimeoutListener onTimeout = nullptr, OnReceiveErrorListener onError = nullptr, std::unique_ptr<Timeout> timeout = nullptr);

void startTransaction(const char *idTag, OnReceiveConfListener onConf = nullptr, OnAbortListener onAbort = nullptr, OnTimeoutListener onTimeout = nullptr, OnReceiveErrorListener onError = nullptr, std::unique_ptr<Timeout> timeout = nullptr);

void stopTransaction(OnReceiveConfListener onConf = nullptr, OnAbortListener onAbort = nullptr, OnTimeoutListener onTimeout = nullptr, OnReceiveErrorListener onError = nullptr, std::unique_ptr<Timeout> timeout = nullptr);

/*
 * Access information about the internal state of the library
 */

int getTransactionId(); //returns the ID of the current transaction. Returns -1 if called before or after an transaction

bool ocppPermitsCharge();

bool isAvailable(); //if the charge point is operative or inoperative

/*
 * Session management
 *
 * A session begins when the EV user is authenticated against the OCPP system and intends to start the charging process.
 * The session ends as soon as the authentication expires or as soon as the EV user does not intend to charge anymore.
 * 
 * A session means that the EVSE does not need further input from the EV user or OCPP backend to start a transaction.
 */

void beginSession(const char *idTag);

void endSession();

bool isInSession();

const char *getSessionIdTag();

/*
 * Configure the device management
 */

#if defined(AO_CUSTOM_UPDATER) || defined(AO_CUSTOM_WS)
#include <ArduinoOcpp/Tasks/FirmwareManagement/FirmwareService.h>
// You need to configure this object if FW updates are relevant for you. This project already
// brings a simple configuration for the ESP32 and ESP8266 for prototyping purposes, however
// for the productive system you will have to develop a configuration targeting the specific
// OCPP backend.
// See  ArduinoOcpp/Tasks/FirmwareManagement/FirmwareService.h 
ArduinoOcpp::FirmwareService *getFirmwareService();
#endif

#if defined(AO_CUSTOM_DIAGNOSTICS) || defined(AO_CUSTOM_WS)
#include <ArduinoOcpp/Tasks/Diagnostics/DiagnosticsService.h>
// This library implements the OCPP messaging side of Diagnostics, but no logging or the
// log upload to your backend.
// To integrate Diagnostics, see ArduinoOcpp/Tasks/Diagnostics/DiagnosticsService.h
ArduinoOcpp::DiagnosticsService *getDiagnosticsService();
#endif

#endif
